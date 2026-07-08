//---------------------------------------------------------------------------
//! @file   ModelCache.cpp
//! @brief  3Dモデルキャッシュ
//---------------------------------------------------------------------------
#include "ModelCache.h"
#include <filesystem>
#include <iomanip>

#include <meshoptimizer/src/meshoptimizer.h>

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
ModelCache::ModelCache(std::string_view path)
{
    model_path_ = path;

    //　対応するキャッシュファイルのパスを取得
    {
        std::array<char, 1024> temporary_path;
        GetTempPath(static_cast<DWORD>(sizeof(temporary_path)), temporary_path.data());

        // 現在の日付を文字列に変換
        SYSTEMTIME time;
        GetLocalTime(&time);

        std::ostringstream oss;
        oss << time.wYear << "-" << std::setw(2) << std::setfill('0') << time.wMonth << "-" << std::setw(2) << std::setfill('0') << time.wDay;
        std::string date_str = oss.str();

        // キャッシュファイルのパスを設定
        model_cache_path_ = std::string(temporary_path.data()) + "BaseProject" + date_str + "/" + std::string(path) + ".cache ";
    }
}

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
ModelCache::~ModelCache()
{
    // 頂点バッファ
    if(handle_vb_ != -1) {
        DxLib::DeleteVertexBuffer(handle_vb_);
    }

    // インデックスバッファ
    if(handle_ib_ != -1) {
        DxLib::DeleteIndexBuffer(handle_ib_);
    }
}

//---------------------------------------------------------------------------
//! モデルキャッシュを保存
//---------------------------------------------------------------------------
bool ModelCache::save(int mv1_handle) const
{
    std::vector<VECTOR> varray;    // 頂点配列
    std::vector<u32>    iarray;    // インデックス配列

    // 指定のパスにモデルを保存する
    // DxLib::MV1SaveModelToMV1FileWithStrLen(handle_, path.data(), path.size(), MV1_SAVETYPE_NORMAL);

    //--------------------------------------------------------------
    // MV1モデルから頂点形状を抽出する
    //--------------------------------------------------------------
    {
        bool is_transform = true;
        DxLib::MV1SetupReferenceMesh(mv1_handle, -1, is_transform, true);                   // 参照用メッシュのセットアップ
        auto poly_list = DxLib::MV1GetReferenceMesh(mv1_handle, -1, is_transform, true);    // 参照用メッシュを取得する

        // 頂点インデックス配列を抽出
        varray.resize(0);
        for(s32 i = 0; i < poly_list.VertexNum; ++i) {
            auto vertex = poly_list.Vertexs[i];

            varray.push_back(vertex.Position);
        }

        // インデックス配列を抽出
        iarray.resize(0);
        for(s32 i = 0; i < poly_list.PolygonNum; ++i) {
            auto polygon = poly_list.Polygons[i];

            // 縮退三角形を検出したら破棄
            u32 i0 = polygon.VIndex[0];
            u32 i1 = polygon.VIndex[1];
            u32 i2 = polygon.VIndex[2];

            auto v0 = cast(varray[i0]);
            auto v1 = cast(varray[i1]);
            auto v2 = cast(varray[i2]);
            auto c  = cross(v2 - v1, v0 - v1);
            if(dot(c, c).x < 0.00001f) {
                continue;
            }

            // 登録
            iarray.push_back(i0);
            iarray.push_back(i1);
            iarray.push_back(i2);
        }

        DxLib::MV1TerminateReferenceMesh(mv1_handle, -1, false, true);    // 参照用メッシュの後始末
    }

    //----------------------------------------------------------
    // 頂点データー重複とインデックス最適化
    //----------------------------------------------------------
    auto merge_duplicated_vertex = [&]() {
        std::vector<u32> remap(varray.size());    // 最長の頂点数分だけインデックスを確保

        // [meshoptimizer] 重複頂点を結合
        size_t total_vertex_count = meshopt_generateVertexRemap(remap.data(), iarray.data(), iarray.size(), varray.data(), varray.size(), sizeof(VECTOR));

        // [meshoptimizer] インデックスバッファを結合後の頂点でつけ直す
        meshopt_remapIndexBuffer(iarray.data(), iarray.data(), iarray.size(), remap.data());

        // [meshoptimizer] 使用している頂点のみで詰め直す
        meshopt_remapVertexBuffer(varray.data(), varray.data(), varray.size(), sizeof(VECTOR), remap.data());
        varray.resize(total_vertex_count);
    };

    merge_duplicated_vertex();

    //----------------------------------------------------------
    // LOD生成
    //----------------------------------------------------------
    constexpr u32    LOD_COUNT = 8;
    std::vector<u32> lods[LOD_COUNT];

    lods[0] = iarray;

    for(size_t i = 1; i < LOD_COUNT; ++i) {
        auto& lod = lods[i];

        f64    threshold          = pow(0.7, static_cast<f32>(i));
        size_t target_index_count = static_cast<size_t>(iarray.size() * threshold) / 3 * 3;
        f32    target_error       = 1e-2f;

        auto& source = lods[0];

        if(source.size() < target_index_count)
            target_index_count = source.size();

        lod.resize(source.size());

        size_t result_size = meshopt_simplify(lod.data(),
                                              source.data(),
                                              source.size(),
                                              reinterpret_cast<const float*>(varray.data()),
                                              varray.size(),
                                              sizeof(VECTOR),
                                              target_index_count,
                                              target_error);

        lod.resize(result_size);
    }
    iarray = lods[0];
    //iarray = lods[4];    // LOD4を採用

    // 頂点データー重複とインデックス最適化
    merge_duplicated_vertex();

    //----------------------------------------------------------
    // 頂点の最適化
    //----------------------------------------------------------
    // [meshoptimizer] 頂点キャッシュ最適化
    meshopt_optimizeVertexCache(iarray.data(), iarray.data(), iarray.size(), varray.size());

    // [meshoptimizer] オーバードロー最適化
    meshopt_optimizeOverdraw(iarray.data(), iarray.data(), iarray.size(), &varray[0].x, varray.size(), sizeof(VECTOR), 1.0f);

    // [meshoptimizer] 頂点フェッチ最適化
    meshopt_optimizeVertexFetch(varray.data(), iarray.data(), iarray.size(), varray.data(), varray.size(), sizeof(VECTOR));

    //----------------------------------------------------------
    // 縮退三角形の存在を再チェック
    //----------------------------------------------------------
    for(u32 i = 0; i < iarray.size(); i += 3) {
        auto i0 = iarray[i + 0];
        auto i1 = iarray[i + 1];
        auto i2 = iarray[i + 2];
        auto v0 = cast(varray[i0]);
        auto v1 = cast(varray[i1]);
        auto v2 = cast(varray[i2]);
        auto c  = cross(v2 - v1, v0 - v1);

        // 縮退三角形が検出された
        if(dot(c, c).x < 0.00001f) {
            // 対象の三角形を削除する
            iarray.erase(iarray.begin() + i);
            iarray.erase(iarray.begin() + i);
            iarray.erase(iarray.begin() + i);
            i -= 3;
        }
    }

    //----------------------------------------------------------
    // ディレクトリを作成
    //----------------------------------------------------------
    {
        auto directory_name = std::filesystem::path(model_cache_path_).remove_filename();

        // フォルダ階層をまとめて作成
        // エラーコードを受け取ると例外を送出しない
        std::error_code error_code;
        std::filesystem::create_directories(directory_name, error_code);
    }

    //----------------------------------------------------------
    // 保存
    //----------------------------------------------------------
    std::string   file_path(model_cache_path_);    // null終端文字列に変換
    std::ofstream stream(file_path.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    if(!stream.is_open()) {
        return false;
    }

    // ファイルヘッダー
    ModelCache::Desc desc{};
    {
        desc.file_version_ = ModelCache::VERSION;                  // ファイルバーション
        desc.frame_count_  = DxLib::MV1GetFrameNum(mv1_handle);    // 関節数
        desc.vertex_count_ = static_cast<u32>(varray.size());      // 頂点数
        desc.index_count_  = static_cast<u32>(iarray.size());      // インデックス数

        stream.write(reinterpret_cast<char*>(&desc), sizeof(desc));
    }
    // 頂点配列
    stream.write(reinterpret_cast<char*>(varray.data()), desc.vertex_count_ * sizeof(VECTOR));

    // インデックス配列
    stream.write(reinterpret_cast<char*>(iarray.data()), desc.index_count_ * sizeof(u32));

    return true;
}

//---------------------------------------------------------------------------
//! モデルキャッシュを読み込み
//---------------------------------------------------------------------------
bool ModelCache::load()
{
    // ロードされたバイナリー
    std::vector<std::byte> binary;

    //----------------------------------------------------------
    // キャッシュファイルを読み込み
    //----------------------------------------------------------
    {
        auto handle = DxLib::FileRead_fullyLoad_WithStrLen(model_cache_path_.data(), model_cache_path_.size());
        if(handle == -1) {
            return false;
        }
        auto* p    = DxLib::FileRead_fullyLoad_getImage(handle);
        auto  size = DxLib::FileRead_fullyLoad_getSize(handle);

        binary.resize(size);
        memcpy(binary.data(), p, size);

        // 一時領域を解放
        DxLib::FileRead_fullyLoad_delete(handle);
    }

    //----------------------------------------------------------
    // メモリ領域から取り出し
    //----------------------------------------------------------
    {
        uintptr_t p = reinterpret_cast<uintptr_t>(binary.data());

        // ファイル情報
        desc_ = *reinterpret_cast<ModelCache::Desc*>(p);

        // ファイルバージョンが異なっていた場合はキャッシュクリア
        if(desc_.file_version_ != ModelCache::VERSION) {
            // エラーコードを受け取ると例外を送出しない
            std::error_code error_code;
            std::filesystem::remove(model_cache_path_, error_code);
            return false;
        }

        p += sizeof(ModelCache::Desc);

        // 頂点配列
        vertices_.resize(desc_.vertex_count_);
        memcpy(vertices_.data(), reinterpret_cast<void*>(p), sizeof(VECTOR) * desc_.vertex_count_);
        p += sizeof(VECTOR) * desc_.vertex_count_;

        // インデックス配列
        indices_.resize(desc_.index_count_);
        memcpy(indices_.data(), reinterpret_cast<void*>(p), sizeof(u32) * desc_.index_count_);
        p += sizeof(u32) * desc_.index_count_;
    }

    //----------------------------------------------------------
    // 頂点バッファとインデックスバッファを作成
    //----------------------------------------------------------
    {
        // DXライブラリ形式の頂点データーを一時的に作成
        std::vector<VERTEX3D> varray;
        std::vector<u32>      iarray;

        for(DxLib::VECTOR& position : vertices_) {
            VERTEX3D v{};

            v.pos = position;
            v.dif = DxLib::GetColorU8(255, 255, 0, 255);

            varray.emplace_back(std::move(v));
        }
        for(u32 i = 0; i < indices_.size(); i += 3) {
            u32 a = indices_[i + 0];
            u32 b = indices_[i + 1];
            u32 c = indices_[i + 2];

            // ワイヤーフレーム描画にするために三角形abcインデックスを a-b, b-c, c-a という順に接続する
            iarray.push_back(a);
            iarray.push_back(b);
            iarray.push_back(b);
            iarray.push_back(c);
            iarray.push_back(c);
            iarray.push_back(a);
        }

        // 頂点バッファとインデックスバッファを作成
        handle_vb_ = DxLib::CreateVertexBuffer(static_cast<s32>(varray.size()), DX_VERTEX_TYPE_NORMAL_3D);
        handle_ib_ = DxLib::CreateIndexBuffer(static_cast<s32>(iarray.size()), DX_INDEX_TYPE_32BIT);

        // バッファにデーターを転送
        DxLib::SetVertexBufferData(0, varray.data(), static_cast<s32>(varray.size()), handle_vb_);
        DxLib::SetIndexBufferData(0, iarray.data(), static_cast<s32>(iarray.size()), handle_ib_);
    }

    is_valid_ = true;
    return true;
}

//---------------------------------------------------------------------------
//! ファイル情報を取得します
//---------------------------------------------------------------------------
const ModelCache::Desc& ModelCache::desc() const
{
    return desc_;
}

//---------------------------------------------------------------------------
//! 頂点配列を取得
//---------------------------------------------------------------------------
const std::vector<VECTOR>& ModelCache::vertices() const
{
    return vertices_;
}

//---------------------------------------------------------------------------
//! インデックス配列を取得
//---------------------------------------------------------------------------
const std::vector<u32>& ModelCache::indices() const
{
    return indices_;
}

//---------------------------------------------------------------------------
// 初期化が正しく成功しているかどうか
//---------------------------------------------------------------------------
bool ModelCache::isValid() const
{
    return is_valid_;
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void ModelCache::render(const matrix& mat_world) const
{
    if(!isValid()) {
        return;
    }

    // ワールド行列を設定
    MATRIX matrix = cast(mat_world);
    SetTransformToWorld(&matrix);

    //----------------------------------------------------------
    // ジオメトリを描画
    // 頂点バッファの利用でCPU負荷を大幅に削減できる
    //----------------------------------------------------------
    if constexpr(false) {    // デバッグ描画を利用した描画

        for(size_t i = 0; i < indices_.size(); i += 3) {
            auto i0 = indices_[i + 0];
            auto i1 = indices_[i + 1];
            auto i2 = indices_[i + 2];

            DrawTriangle3D(vertices_[i0], vertices_[i1], vertices_[i2], GetColor(255, 255, 0), false);
        }
    }
    else {    // 頂点バッファを利用した描画

        SetUseLighting(false);    // 照明OFF
        DrawPrimitiveIndexed3D_UseVertexBuffer(handle_vb_, handle_ib_, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, false);
        SetUseLighting(true);
    }

    // 単位行列を設定して元に戻す
    MATRIX mat_identity = MGetIdent();
    SetTransformToWorld(&mat_identity);
}

//---------------------------------------------------------------------------
//!  キャッシュファイルが存在するかチェック
//---------------------------------------------------------------------------
bool ModelCache::isExist() const
{
    return std::filesystem::exists(model_cache_path_);
}
