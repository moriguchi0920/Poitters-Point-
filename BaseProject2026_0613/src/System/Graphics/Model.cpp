//---------------------------------------------------------------------------
//! @file   Model.cpp
//! @brief  3Dモデル
//---------------------------------------------------------------------------
#include "Model.h"
#include "ResourceModel.h"
#include "ModelCache.h"
#include "Shader.h"
#include "Animation.h"

namespace {

//! モデルリソースプール
std::unordered_map<std::string, std::shared_ptr<ResourceModel>> resource_model_pool;

}    // namespace

//===========================================================================
//! [DxLib] モデル行列キャッシュ
//===========================================================================
class DxLib_MV1MatrixCache
{
public:
    //! デフォルトコンストラクタ
    DxLib_MV1MatrixCache() = default;

    //! コンストラクタ
    //! @param  [in]    MV1モデルハンドル
    DxLib_MV1MatrixCache(int mv1_handle) { initialize(mv1_handle); }

    //! 初期化
    //! @param  [in]    MV1モデルハンドル
    bool initialize(int mv1_handle)
    {
        mv1_handle_ = mv1_handle;

        // 配列を確保
        auto triangle_list_count = DxLib::MV1GetTriangleListNum(mv1_handle);

        for(s32 i = 0; i < 2; ++i) {    // ダブルバッファの数
            tlists_[i].resize(triangle_list_count);

            for(s32 t = 0; t < triangle_list_count; t++) {
                auto& tlist = tlists_[i][t];

                //--------------------------------------------------------------
                // 頂点座標保存用のStreamOutバッファを作成
                //--------------------------------------------------------------
                // [DxLib] 内部のDirect3Dデバイスを取得
                auto* d3d_device = reinterpret_cast<ID3D11Device*>(const_cast<void*>(DxLib::GetUseDirect3D11Device()));

                u32 vertex_count  = MV1GetTriangleListVertexNum(mv1_handle, t);
                vertex_count     *= 3;    // インデックスバッファを単純な三角形に展開した場合に最大3倍の頂点になる可能性
                {
                    // DirectX11では構造化バッファを頂点バッファで利用できない
                    // 代わりにByteAddressBufferでバインドします。
                    D3D11_BUFFER_DESC desc{
                        .ByteWidth           = sizeof(float4) * vertex_count,                            // バッファーのサイズ (バイト単位)
                        .Usage               = D3D11_USAGE_DEFAULT,                                      // メモリの配置場所 (読み取りと書き込み方法)
                        .BindFlags           = D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_SHADER_RESOURCE,    // バインド用途
                        .CPUAccessFlags      = 0,                                                        // CPUアクセス許可フラグ (殆どの場合0)
                        .MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,               // ByteAddressBuffer
                        .StructureByteStride = 0,                                                        // 構造化バッファの構造体あたりの幅
                    };

                    HRESULT hr = d3d_device->CreateBuffer(&desc, nullptr, &tlist.d3d_position_buffer_);
                    if(hr != S_OK) {
                        return false;
                    }
                }

                // StreamOutバッファ用のShaderResourceViewを作成(ByteAddressBuffer)
                {
                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3d_srv;

                    D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
                    desc.Format                = DXGI_FORMAT_R32_TYPELESS;        // float型 (TYPELESS指定が必要)
                    desc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;    // BufferEx
                    desc.BufferEx.FirstElement = 0;                               // 最初の要素インデックス番号
                    desc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;     // ByteAddressBuffer
                    desc.BufferEx.NumElements  = vertex_count * 4;                // float * 4 * 頂点数

                    HRESULT hr = d3d_device->CreateShaderResourceView(tlist.d3d_position_buffer_.Get(), &desc, &d3d_srv);
                    if(hr != S_OK) {
                        return false;
                    }
                    tlist.d3d_srv_position_buffer_ = std::move(d3d_srv);
                }
            }
        }
        return true;
    }

    //! ダブルバッファを交換
    void flip() { std::swap(tlists_[0], tlists_[1]); }

    //! StreamOut用座標バッファを取得
    ID3D11Buffer* position_buffer(u32 tlist_index) const
    {
        // [0] = 現在のバッファ
        return tlists_[0][tlist_index].d3d_position_buffer_.Get();
    }

    //! StreamOut用座標バッファを取得
    ID3D11ShaderResourceView* srv(u32 tlist_index) const
    {
        // [1] = 1フレーム前のバッファ
        return tlists_[1][tlist_index].d3d_srv_position_buffer_.Get();
    }

private:
    struct Tlist
    {
        Microsoft::WRL::ComPtr<ID3D11Buffer>             d3d_position_buffer_;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3d_srv_position_buffer_;
    };

    int                mv1_handle_ = 0;    //!< [DxLib] 対象のMV1モデルハンドル
    std::vector<Tlist> tlists_[2];         //!< TListごとの情報
};

//---------------------------------------------------------------------------
//! 読み込み
//---------------------------------------------------------------------------
bool Model::load(std::string_view path)
{
    //----------------------------------------------------------
    // モデルリソース読み込み
    //----------------------------------------------------------
    {
        std::string resource_path = std::string(path);

        auto it = resource_model_pool.find(resource_path);
        if(it == resource_model_pool.end()) {    // 新規登録
            resource_model_pool[resource_path] = std::make_shared<ResourceModel>(path);
        }

        // 共有
        resource_model_ = resource_model_pool[resource_path];
    }

    // 以前ロードしたデータは削除する
    if(animation_) {
        animation_->bindModel(nullptr);
    }

    // [DxLib] MV1モデルを解放 (複製されたハンドル)
    if(mv1_handle_ != -1) {
        MV1DeleteModel(mv1_handle_);
        mv1_handle_ = -1;
    }

    // 初期化を要求
    need_initialize_ = true;

    // 初回のみ読み込み
    if(shader_vs_ == nullptr || shader_ps_ == nullptr) {
        //----------------------------------------------------------
        // シェーダーを読み込み
        //----------------------------------------------------------
        // 頂点シェーダー
        // DX_MV1_VERTEX_TYPE_NUM 個のシェーダーバリエーションを生成
        shader_vs_ = std::make_shared<ShaderVs>("data/Shader/vs_model", VS_VARIANT_COUNT);

        // ピクセルシェーダー
        shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_model", PS_VARIANT_COUNT);

        //----------------------------------------------------------
        // デフォルトテクスチャを読み込み
        //----------------------------------------------------------
        tex_null_white_  = std::make_shared<Texture>("data/System/null_white.dds");
        tex_null_black_  = std::make_shared<Texture>("data/System/null_black.dds");
        tex_null_normal_ = std::make_shared<Texture>("data/System/null_normal.dds");
    }

    // 速度バッファ生成用のStreamOutジオメトリシェーダーを作成
    {
        shader_gs_streamout_position_      = std::make_shared<ShaderGs>("data/Shader/gs_model_streamout_position", 1);
        shader_gs_composite_prev_position_ = std::make_shared<ShaderGs>("data/Shader/gs_model_composite_prev_position", 1);

        // StreamOut用のバッファ出力レイアウト
        // ストリーム番号, "出力要素の型", 出力要素のインデックス, 書き込みを開始するエントリのコンポーネント(0-1), コンポーネントの数(1-4), ストリーム出力バッファースロット番号(0-3)
        D3D11_SO_DECLARATION_ENTRY so_declaration_entry[]{
            {0, "POSITION", 0, 0, 4, 0},
        };
        std::array<u32, 1> strides = {sizeof(float) * 4};

        // [DxLib] 内部のDirect3Dデバイスを取得
        auto* d3d_device = reinterpret_cast<ID3D11Device*>(const_cast<void*>(DxLib::GetUseDirect3D11Device()));

        auto [shader_binary, shader_binary_size] = shader_gs_streamout_position_->shader_bytecode();

        HRESULT hr = d3d_device->CreateGeometryShaderWithStreamOutput(
            shader_binary,                                        // [in]  シェーダーバイナリ
            shader_binary_size,                                   // [in]  シェーダーバイナリのサイズ
            so_declaration_entry,                                 // [in]  StreamOut用のバッファ出力レイアウト
            static_cast<u32>(std::size(so_declaration_entry)),    // [in]  StreamOut用のバッファ出力レイアウト配列数
            strides.data(),                                       // [in]  バッファーストライドの配列
            static_cast<u32>(strides.size()),                     // [in]  バッファーストライドの配列数
            D3D11_SO_NO_RASTERIZED_STREAM,                        // [in]  ラスタライザーステージに送信されるストリームのインデックス番号
            nullptr,                                              // [in]  ID3D11ClassLinkageのポインタ
            &d3d_shader_gs_streamout_position_);                  // [out] D3Dジオメトリシェーダー

        if(hr != S_OK) {
            return false;
        }
    }

    return resource_model_->isValid();
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void Model::update([[maybe_unused]] f32 dt)
{
    //! 速度バッファ用に頂点キャッシュをflipします。
    if(mv1_matrix_cache_)
        mv1_matrix_cache_->flip();
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void Model::render(ShaderVs* override_vs, ShaderPs* override_ps)
{
    if(!resource_model_)
        return;

    if(!resource_model_->isActive()) {
        // ロードが終わっていない間は軽量モデルキャッシュ側を描画
        auto* model_cache = resource_model_->modelCache();
        model_cache->render(mat_world_);

        return;
    }

    // ロード終了していたらハンドルを複製
    on_initialize();

    for(s32 frame = 0; frame < frameCount(); ++frame) {
        renderByFrame(frame, override_vs, override_ps);
    }
}

//---------------------------------------------------------------------------
//! フレーム番号指定で描画
//---------------------------------------------------------------------------
void Model::renderByFrame(s32 frame_index, ShaderVs* override_vs, ShaderPs* override_ps)
{
    if(!resource_model_)
        return;

    if(!resource_model_->isActive())
        return;

    // ロード終了していたらハンドルを複製
    on_initialize();

    // フレーム番号が有効範囲外
    if(MV1GetFrameNum(mv1_handle_) <= frame_index) {
        return;
    }

    // ワールド行列を設定
    MV1SetMatrix(mv1_handle_, mat_world_);

    // シェーダーを使わない場合はDxLib関数を直接実行
    if(!use_shader_) {
        MV1DrawFrame(mv1_handle_, frame_index);
        return;
    }

    //--------------------------------------------------
    // テクスチャ設定の上書き
    //--------------------------------------------------
    bool override_normalmap = false;

    if(overridedTextures_[static_cast<s32>(Model::TextureType::Diffuse)]) {
        SetUseTextureToShader(0, *overridedTextures_[static_cast<s32>(Model::TextureType::Diffuse)]);
    }
    if(overridedTextures_[static_cast<s32>(Model::TextureType::Normal)]) {
        SetUseTextureToShader(1, *overridedTextures_[static_cast<s32>(Model::TextureType::Normal)]);
        override_normalmap = true;    // 法線マップを使用
    }
    if(overridedTextures_[static_cast<s32>(Model::TextureType::Specular)]) {
        SetUseTextureToShader(2, *overridedTextures_[static_cast<s32>(Model::TextureType::Specular)]);
    }

    //--------------------------------------------------
    // シェーダーで描画
    //--------------------------------------------------
    ShaderVs* vs = override_vs ? override_vs : shader_vs_.get();
    ShaderPs* ps = override_ps ? override_ps : shader_ps_.get();

    // オリジナルシェーダーを使用をONにする
    MV1SetUseOrigShader(true);

    for(s32 mesh_index = 0; mesh_index < MV1GetFrameMeshNum(mv1_handle_, frame_index); ++mesh_index) {    // フレームに含まれるメッシュの数
        s32 mesh = MV1GetFrameMesh(mv1_handle_, frame_index, mesh_index);                                 // メッシュ番号

        for(s32 tlist_index = 0; tlist_index < MV1GetMeshTListNum(mv1_handle_, mesh); ++tlist_index) {    // メッシュに含まれるトライアングルリストの数
            auto tlist = MV1GetMeshTList(mv1_handle_, mesh, tlist_index);                                 // トライアングルリスト番号

            // トライアングルリストが使用しているマテリアルのインデックスを取得する
            auto material_index = MV1GetTriangleListUseMaterial(mv1_handle_, tlist);

            // 法線マップを使用しているかどうか
            bool use_normalmap = MV1GetMaterialNormalMapTexture(mv1_handle_, material_index) != -1;

            // 法線マップを使用しない場合はNull法線を登録しておく
            if(!use_normalmap && !override_normalmap) {
                SetUseTextureToShader(1, *tex_null_normal_);
            }

            //--------------------------------------------------
            // シェーダーバリエーションを選択
            //--------------------------------------------------
            // 頂点データタイプ(DX_MV1_VERTEX_TYPE_1FRAME 等)
            auto vertex_type = MV1GetTriangleListVertexType(mv1_handle_, tlist);
            u32  variant_vs  = vertex_type;    // DXライブラリの頂点タイプをそのままバリエーション番号に

            //--------------------------------------------------
            // トライアングルリストを描画
            //--------------------------------------------------
            int handle_vs = vs->variant(variant_vs);
            int handle_ps = *ps;

            // シェーダーがない場合はオリジナルシェーダー利用を無効化
            bool shader_enable = (handle_vs != -1) && (handle_ps != -1);
            DxLib::MV1SetUseOrigShader(shader_enable);

            // 頂点シェーダー
            DxLib::SetUseVertexShader(handle_vs);

            // ピクセルシェーダー
            DxLib::SetUsePixelShader(handle_ps);

            //--------------------------------------------------
            // 速度バッファ生成のための2パス描画
            //--------------------------------------------------
            // StreamOutを利用して現在の頂点をバッファに出力保存
            {
                auto* d3d_context = reinterpret_cast<ID3D11DeviceContext*>(const_cast<void*>(DxLib::GetUseDirect3D11DeviceContext()));

                // ジオメトリシェーダーでStreamOutするための出力先を設定
                Microsoft::WRL::ComPtr<ID3D11GeometryShader> old_shader_gs;
                {
                    ID3D11Buffer* stream_out_buffer = mv1_matrix_cache_->position_buffer(tlist);

                    u32 offset[1] = {0};
                    d3d_context->SOSetTargets(1, &stream_out_buffer, offset);

                    d3d_context->GSGetShader(&old_shader_gs, nullptr, nullptr);
                    d3d_context->GSSetShader(d3d_shader_gs_streamout_position_.Get(), nullptr, 0);
                }

                // 描画 (StreamOut用)
                MV1DrawTriangleList(mv1_handle_, tlist);

                // StreamOutをもとに戻す
                {
                    ID3D11Buffer* stream_out_buffer = nullptr;
                    u32           offset[1]         = {0};
                    d3d_context->SOSetTargets(1, &stream_out_buffer, offset);

                    d3d_context->GSSetShader(old_shader_gs.Get(), nullptr, 0);
                }
            }

            // (2) 1フレーム前の(1)を設定してGeometryShaderで頂点合成
            {
                auto* d3d_context = reinterpret_cast<ID3D11DeviceContext*>(const_cast<void*>(DxLib::GetUseDirect3D11DeviceContext()));
                u32   slot        = 0;

                // ジオメトリシェーダーで1フレーム前の頂点座標を合成
                DxLib::SetUseGeometryShader(*shader_gs_composite_prev_position_);

                // 1フレーム前のStreamOutされた頂点バッファをShaderResourceとして設定
                Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> old_srv;
                {
                    ID3D11ShaderResourceView* srv = mv1_matrix_cache_->srv(tlist);
                    d3d_context->GSGetShaderResources(slot, 1, &old_srv);
                    d3d_context->GSSetShaderResources(slot, 1, &srv);
                }

                // 描画 (本来のシーン用)
                MV1DrawTriangleList(mv1_handle_, tlist);

                // もとに戻す
                {
                    d3d_context->GSSetShaderResources(slot, 1, &old_srv);
                    DxLib::SetUseGeometryShader(-1);
                }
            }
        }
    }

    // オリジナルシェーダー使用をOFFにする
    DxLib::MV1SetUseOrigShader(false);

    // 使い終わったらテクスチャ設定を解除
    // 解除しておかないと他のモデル描画に影響あり。
    DxLib::SetUseTextureToShader(0, -1);
    DxLib::SetUseTextureToShader(1, -1);
    DxLib::SetUseTextureToShader(2, -1);
}

//---------------------------------------------------------------------------
//  コンストラクタ
//---------------------------------------------------------------------------
Model::Model()
{
    ref_counter_++;
}

Model::Model(std::string_view path)
{
    load(path);
    ref_counter_++;
}

//---------------------------------------------------------------------------
//  デストラクタ
//---------------------------------------------------------------------------
Model::~Model()
{
    // アニメーションが関連付けられている場合は相手の設定を解除
    if(animation_) {
        animation_->bindModel(nullptr);
    }

    // [DxLib] MV1モデルを解放 (複製されたハンドル)
    if(mv1_handle_ != -1) {
        DxLib::MV1DeleteModel(mv1_handle_);
    }

    // 一番最後のオブジェクトが解放を担当
    ref_counter_--;

    if(ref_counter_ == 0) {
        // シェーダー
        shader_vs_.reset();
        shader_ps_.reset();

        // デフォルトテクスチャ
        tex_null_white_.reset();
        tex_null_black_.reset();
        tex_null_normal_.reset();
    }
}

//---------------------------------------------------------------------------
//! ワールド行列を設定
//---------------------------------------------------------------------------
void Model::setWorldMatrix(const matrix& mat_world)
{
    mat_world_ = mat_world;
    DxLib::MV1SetMatrix(mv1_handle_, cast(mat_world));
}

//---------------------------------------------------------------------------
//! アニメーションを設定
//---------------------------------------------------------------------------
void Model::bindAnimation(Animation* animation)
{
    if(animation == animation_)
        return;

    // 旧モデルを解除
    auto* last = animation_;
    if(last) {
        animation_ = nullptr;
        last->bindModel(nullptr);
    }

    // 新モデルに登録
    animation_ = animation;
    if(animation_) {
        animation_->bindModel(this);
    }
}

//---------------------------------------------------------------------------
//!  既存テクスチャをオーバーライドします
//---------------------------------------------------------------------------
void Model::overrideTexture(Model::TextureType type, std::shared_ptr<Texture>& texture)
{
    overridedTextures_[static_cast<s32>(type)] = texture;
}

//---------------------------------------------------------------------------
//!  テクスチャオーバーライドを解除します
//---------------------------------------------------------------------------
void Model::overrideTexture(Model::TextureType type, nullptr_t)
{
    overridedTextures_[static_cast<s32>(type)] = nullptr;
}

//---------------------------------------------------------------------------
//! モデルのフレーム総数を取得
//---------------------------------------------------------------------------
s32 Model::frameCount()
{
    if(resource_model_->isActive()) {
        // ロード終了していたらハンドルを複製
        on_initialize();

        return DxLib::MV1GetFrameNum(mv1_handle_);
    }

    // ロードが終わっていない間は軽量モデルキャッシュ側を描画
    if(auto* model_cache = resource_model_->modelCache(); model_cache) {
        return model_cache->desc().frame_count_;
    }

    // 非同期読み込みをブロッキングロードにしてすぐに取得
    on_initialize();
    return DxLib::MV1GetFrameNum(mv1_handle_);
}

//---------------------------------------------------------------------------
//! ワールド行列を設定
//---------------------------------------------------------------------------
matrix Model::worldMatrix() const
{
    return mat_world_;
}

//---------------------------------------------------------------------------
//! [DxLib] MV1ハンドルを取得
//---------------------------------------------------------------------------
Model::operator int()
{
    // 強制的にハンドルを複製 (ブロッキングロードに切り替わる)
    on_initialize();

    return mv1_handle_;
}

//---------------------------------------------------------------------------
//! ファイルパスを取得
//---------------------------------------------------------------------------
const std::wstring& Model::path() const
{
    return path_;
}

//---------------------------------------------------------------------------
//! 初期化が正しく成功しているかどうかを取得
//---------------------------------------------------------------------------
bool Model::isValid() const
{
    if(!resource_model_)
        return false;

    return resource_model_->isValid();
}

//---------------------------------------------------------------------------
//! 利用可能な状態かどうか取得
//---------------------------------------------------------------------------
bool Model::isActive() const
{
    if(!resource_model_)
        return false;

    return resource_model_->isActive();
}

//---------------------------------------------------------------------------
//! モデルリソースを取得
//---------------------------------------------------------------------------
ResourceModel* Model::resource() const
{
    return resource_model_.get();
}

//---------------------------------------------------------------------------
//! 遅延初期化
//---------------------------------------------------------------------------
void Model::on_initialize()
{
    if(need_initialize_ == false)
        return;

    if(mv1_handle_ == -1) {
        if(resource_model_)
            mv1_handle_ = DxLib::MV1DuplicateModel(*resource_model_);    // ハンドルを複製

        // 行列キャッシュを初期化
        mv1_matrix_cache_ = std::make_unique<DxLib_MV1MatrixCache>(mv1_handle_);

        need_initialize_ = false;
    }
}
