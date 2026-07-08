//---------------------------------------------------------------------------
//! @file   Model.h
//! @brief  3Dモデル
//---------------------------------------------------------------------------
#pragma once

class ModelCache;       // 3Dモデルキャッシュ
class ResourceModel;    // モデルリソース
class Animation;        // アニメーション

class DxLib_MV1MatrixCache;    // [DxLib] モデル行列キャッシュ

//===========================================================================
//! 3Dモデル
//===========================================================================
class Model final : noncopyable, nonmovable
{
public:
    //! 頂点シェーダーのバリエーション数
    static constexpr u32 VS_VARIANT_COUNT = DX_MV1_VERTEX_TYPE_NUM;

    //! ピクセルシェーダーのバリエーション数
    static constexpr u32 PS_VARIANT_COUNT = 1;

    //! テクスチャの種類
    enum class TextureType
    {
        Diffuse,      //!< ディフューズ
        Specular,     //!< スペキュラー
        AO,           //!< Ambient Occlusion
        Normal,       //!< 法線マップ
        Albedo,       //!< アルベド
        Roughness,    //!< ラフネス
        Metalness,    //!< メタルネス(Metallic)
        //----
        CountMax,    //!< 定義個数
    };

    //----------------------------------------------------------
    //! @name   初期化
    //----------------------------------------------------------
    //@{

    //! デフォルトコンストラクタ
    Model();

    //  コンストラクタ
    //! @param [in] path    ファイルパス
    Model(std::string_view path);

    // デストラクタ
    virtual ~Model();

    //  読み込み
    //! @param [in] path    ファイルパス
    //! @retval true    成功(正常終了)
    //! @retval false   失敗(エラー終了)
    bool load(std::string_view path);

    //@}
    //----------------------------------------------------------
    //! @name   描画
    //----------------------------------------------------------
    //@{

    // 更新
    //! @param  [in]    dt  経過時間(単位:秒)
    //! @note 速度バッファ生成用の1フレーム前の頂点を更新します
    void update(f32 dt);

    //  描画
    //! @param  [in]    override_vs  上書きする頂点シェーダー (nullptrで無効化)
    //! @param  [in]    override_ps  上書きするピクセルシェーダー (nullptrで無効化)
    void render(ShaderVs* override_vs = nullptr, ShaderPs* override_ps = nullptr);

    //  フレーム番号指定で描画
    //! @param  [in]    frame_index フレーム番号
    //! @param  [in]    override_vs 上書きする頂点シェーダー (nullptrで無効化)
    //! @param  [in]    override_ps 上書きするピクセルシェーダー (nullptrで無効化)
    //! @note フレームの総数は frameCount() で取得することができます。
    void renderByFrame(s32 frame_index, ShaderVs* override_vs = nullptr, ShaderPs* override_ps = nullptr);

    //@}
    //----------------------------------------------------------
    //! @name   設定
    //----------------------------------------------------------
    //@{

    //! ワールド行列を設定
    void setWorldMatrix(const matrix& mat_world);

    //! シェーダーを使うかどうかを設定
    void useShader(bool use) { use_shader_ = use; }

    //  アニメーションを設定
    //! @param  [in]    animation   関連付けるアニメーション(nullptrの場合は解除する)
    void bindAnimation(Animation* animation);

    //  既存テクスチャをオーバーライドします
    //! @param  [in]    type    テクスチャの種類
    //! @param  [in]    texture オーバーライドするテクスチャ(nullptrでオーバーライド解除)
    void overrideTexture(Model::TextureType type, std::shared_ptr<Texture>& texture);

    //!  テクスチャオーバーライドを解除します
    void overrideTexture(Model::TextureType type, nullptr_t);

    //@}
    //----------------------------------------------------------
    //! @name   取得
    //----------------------------------------------------------
    //@{

    // モデルのフレーム総数を取得
    s32 frameCount();

    // ワールド行列を設定
    matrix worldMatrix() const;

    // [DxLib] MV1ハンドルを取得
    operator int();

    // ファイルパスを取得
    const std::wstring& path() const;

    // 初期化が正しく成功しているかどうかを取得
    bool isValid() const;

    // 利用可能な状態かどうか取得
    //! @note   利用可能になっていない状態でDxLib MV1関連の関数を呼ぶと非同期ロードがブロッキングされます
    bool isActive() const;

    // モデルリソースを取得
    ResourceModel* resource() const;

    //@}
private:
    // 遅延初期化
    void on_initialize();

private:
    std::shared_ptr<ResourceModel>        resource_model_;                     //!< モデルリソース
    std::unique_ptr<DxLib_MV1MatrixCache> mv1_matrix_cache_;                   //!< [DxLib] 行列キャッシュ
    int                                   mv1_handle_ = -1;                    //!< [DxLib] MV1モデルハンドル
    std::wstring                          path_;                               //!< ファイルパス
    bool                                  use_shader_ = true;                  //!< シェーダーを使うかどうか
    matrix                                mat_world_  = matrix::identity();    //!< ワールド行列
    Animation*                            animation_  = nullptr;               //!< 関連付けられているアニメーション

    bool need_initialize_ = true;    //!< 初期化要求フラグ true:初期化が必要 false:初期化済または完了で不要

    //! 上書きするテクスチャ
    std::array<std::shared_ptr<Texture>, static_cast<s32>(Model::TextureType::CountMax)> overridedTextures_;

    //----------------------------------------------------------
    //! @name   共用で使用するリソース
    //----------------------------------------------------------
    //@{

    static inline u32                            ref_counter_ = 0;                      //!< 参照カウンター
    static inline std::shared_ptr<ShaderVs>      shader_vs_;                            //!< 頂点シェーダー
    static inline std::shared_ptr<ShaderPs>      shader_ps_;                            //!< ピクセルシェーダー
    static inline std::shared_ptr<ShaderGs>      shader_gs_streamout_position_;         //!< ジオメトリシェーダー(頂点出力)
    static inline std::shared_ptr<ShaderGs>      shader_gs_composite_prev_position_;    //!< ジオメトリシェーダー(頂点合成)
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> d3d_shader_gs_streamout_position_;     //!< [D3D11]ストリーム出力用 (頂点出力)
    static inline std::shared_ptr<Texture>       tex_null_white_;                       //!< 白Nullテクスチャ (1,1,1,1)
    static inline std::shared_ptr<Texture>       tex_null_black_;                       //!< 黒Nullテクスチャ (0,0,0,1)
    static inline std::shared_ptr<Texture>       tex_null_normal_;                      //!< 法線Nullテクスチャ

    //@}
};
