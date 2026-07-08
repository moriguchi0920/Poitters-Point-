//---------------------------------------------------------------------------
//! @file   ModelCache.h
//! @brief  3Dモデルキャッシュ
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! 3Dモデルキャッシュ
//===========================================================================
class ModelCache
{
public:
    //! モデルキャッシュのバージョン
    static constexpr u32 VERSION = 3;

    //--------------------------------------------------------------
    //! ファイル情報
    //--------------------------------------------------------------
    struct Desc
    {
        u32 file_version_ = ModelCache::VERSION;    //!< バージョン番号
        u32 frame_count_  = 0;                      //!< フレーム(関節)数
        u32 vertex_count_ = 0;                      //!< 頂点数
        u32 index_count_  = 0;                      //!< インデックス数
    };

    //----------------------------------------------------------
    //! @name   初期化
    //----------------------------------------------------------
    //@{

    //  コンストラクタ
    //! @param  [in]    path    モデルファイルパス
    ModelCache(std::string_view path);

    //  デストラクタ
    virtual ~ModelCache();

    //  モデルキャッシュを保存
    bool save(int mv1_handle) const;

    //  モデルキャッシュへ読み込み
    bool load();

    //! ファイル情報を取得します
    const ModelCache::Desc& desc() const;

    //! 頂点配列を取得
    const std::vector<VECTOR>& vertices() const;

    //! インデックス配列を取得
    const std::vector<u32>& indices() const;

    // 初期化が正しく成功しているかどうか
    bool isValid() const;

    //@}
    //----------------------------------------------------------
    //! @name 操作
    //----------------------------------------------------------
    //@{

    //  描画
    //! @param  [in]    mat_world   ワールド行列
    void render(const matrix& mat_world) const;

    //  キャッシュファイルが存在するかチェック
    //! @param  [in]    model_path    モデルファイルパス
    bool isExist() const;

    //@}

private:
    ModelCache::Desc    desc_     = {};       //!< ファイル情報
    std::atomic<bool>   is_valid_ = false;    //!< 初期化が正しく成功しているかどうか
    std::string         model_path_;          //!< モデルのファイルパス
    std::string         model_cache_path_;    //!< モデルキャッシュのファイルパス
    std::vector<VECTOR> vertices_;            //!< 頂点配列
    std::vector<u32>    indices_;             //!< インデックス配列
    int                 handle_vb_ = -1;      //!< [DxLib] 頂点バッファハンドル
    int                 handle_ib_ = -1;      //!< [DxLib] インデックスバッファハンドル
};
