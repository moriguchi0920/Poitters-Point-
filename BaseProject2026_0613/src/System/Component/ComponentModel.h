//---------------------------------------------------------------------------
//! @file   ComponentModel.h
//! @brief  モデルコンポーネント
//---------------------------------------------------------------------------
#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentTransform.h>
#include <System/Graphics/Animation.h>

#include <ImGuizmo/ImGuizmo.h>

USING_PTR(ComponentModel);

//! @brief モデルコンポーネントクラス
class ComponentModel : public Component, public IMatrix<ComponentModel>
{
public:
    BP_COMPONENT_DECL(ComponentModel, u8"Model機能クラス");

    ComponentModel()
    {
        // 複数設定可能とする
        SetStatus(Component::StatusBit::SameType, true);

        // シェーダー利用を標準とします
        model_status_.on(ModelBit::UseShader);

        model_ = std::make_unique<Model>();

        path_.reserve(MAX_PATH);
    }

    ~ComponentModel() {}

    void Construct(ObjectPtr owner)
    {
        auto models = owner->GetComponents<ComponentModel>();
        name_       = "Model" + std::to_string(models.size());

        __super::Construct(owner);
    }

    void Construct(ObjectPtr owner, std::string_view path)
    {
        auto models = owner->GetComponents<ComponentModel>();
        name_       = "Model" + std::to_string(models.size());

        __super::Construct(owner);

        // シェーダー利用を標準とします
        model_status_.on(ModelBit::UseShader);

        if(model_ == nullptr)
            model_ = std::make_unique<Model>();

        Load(path);
    }

    //! @brief モデルロード
    //! @param path モデル名
    void Load(std::string_view path);

    virtual void Init() override;      //!< 初期化
    virtual void Update() override;    //!< 更新
    virtual void Draw() override;      //!< 描画
    virtual void Exit() override;      //!< 終了
    virtual void GUI() override;       //!< GUI

    ComponentModelPtr SetName(const std::string& name)
    {
        name_ = name;
        return SharedThis();
    }

    //------------------------------------------------------------------------
    // @name アニメーション
    //------------------------------------------------------------------------
    //@{

    //! @brief アニメーションデータ
    //! @param anims アニメーション構造リスト
    ComponentModelPtr SetAnimation(const std::vector<Animation::Desc>& anims);

    //! @brief アニメーション再生(同じなら再再生しない)
    //! @param name 再生するアニメーション名
    //! @param loop ループするかどうか(デフォルト:しない)
    //! @param speed 補完秒数(デフォルト:0.2秒)
    //! @param start_time スタートする位置(デフォルト:0.0)
    void PlayAnimationNoSame(std::string_view name, bool loop = false, float blend_time = 0.2f, float start_time = 0.0f);

    //! @brief アニメーション再生
    //! @param name 再生するアニメーション名
    //! @param loop ループするかどうか(デフォルト:しない)
    //! @param speed 補完秒数(デフォルト:0.2秒)
    //! @param start_time スタートする位置(デフォルト:0.0)
    void PlayAnimation(std::string_view name, bool loop = false, float blend_time = 0.2f, float start_time = 0.0f);

    //! @brief 追加アニメーション再生
    //! @param name 追加再生するアニメーション名
    //! @param loop ループするかどうか(デフォルト:しない)
    //! @param speed 補完秒数(デフォルト:0.2秒)
    //! @param start_time スタートする位置(デフォルト:0.0)
    void PlayAnimationEx(std::string_view name, std::string_view node, bool loop = false, float blend_time = 0.2f, float start_time = 0.0f);

    //! @brief アニメーション中かどうか
    //! @retval true : アニメーション中
    bool IsPlaying();

    //! @brief Exアニメーション中かどうか
    //! @retval true : アニメーション中
    bool IsExPlaying(std::string_view node);

    //! @brief ポーズ処理と解除処理
    //! @param active ポーズするかどうか
    void PlayPause(bool active = true);

    //! @brief ポーズ中かどうか
    //! @retval true : ポーズ中
    bool IsPaused();

    //! @brief 利用可能かどうか
    //! @retval true : 利用可能
    bool IsAnimationValid();

    //! @brief 再生アニメーション名
    //! @return アニメーション名
    const std::string_view GetPlayAnimationName();

    //! @brief 前回再生したアニメーション名
    //! @return アニメーション名
    const std::string_view GetOldPlayAnimationName();

    //! @brief 再生経過時間の取得
    //! @return 再生経過時間
    const float GetAnimationTime();

    //@}

    //! @brief モデル取得
    //! @return モデル
    Model* GetModelClass() { return model_.get(); }

    //! @brief モデル取得
    //! @return モデル(DxLib)
    int GetModel()
    {
        if(model_.get() != nullptr)
            return *(model_.get());

        return -1;
    }

    //---------------------------------------------------------------------------
    //! @name ノード関係
    //---------------------------------------------------------------------------
    //@{

    std::vector<std::string_view> GetNodesName();
    std::vector<const char*>      GetNodesNamePChar();
    int                           GetNodeIndex(std::string_view name);

    float3 GetNodePosition(std::string_view name, bool local = false);
    float3 GetNodePosition(int no, bool local = false);

    matrix GetNodeMatrix(std::string_view name, bool local = false);
    matrix GetNodeMatrix(int no, bool local = false);

    //@}

    //---------------------------------------------------------------------------
    //! モデルステータス
    //---------------------------------------------------------------------------
    enum struct ModelBit : u64
    {
        SetFileAlready,        //!< ファイル設定済み
        Initialized,           //!< 初期化済み
        ErrorFileNotFound,     //!< ファイル読み込みエラー
        UseShader,             //!< シェーダーを使用する
        AttachedOtherModel,    //!< 異なるコンポーネントモデルの一部として利用する
        UseModelNodeScale,     //!< アタッチ使用する際に相手のノードのスケールに合わせる
    };

    bool IsValid() const { return model_status_.is(ModelBit::Initialized); }    //!< モデルが読み込まれているか?

    bool UseShader() const { return model_status_.is(ModelBit::UseShader); }    //!< シェーダーを利用するか?

    void UseShader(bool use) { model_status_.set(ModelBit::UseShader, use); }

    bool IsAttachedOtherModel() const { return model_status_.is(ModelBit::AttachedOtherModel); }

    void SetAttachOtherModel(bool link) { model_status_.set(ModelBit::AttachedOtherModel, link); }

    bool Attach(const std::string_view& model, const std::string_view& node, bool use_model_node_scale = false);

    void Detach();

public:
    //---------------------------------------------------------------------------
    //! @name Materialインターフェースの利用するための定義
    //---------------------------------------------------------------------------
    //@{
    struct Material
    {
        friend class ComponentModel;

        Material() {}

        ~Material() {}

        Material(const std::string_view diffuse, const std::string_view normal, const std::string_view roughness, const std::string_view metalness)
        {
            SetDiffuse(diffuse);
            SetNormal(normal);
            SetRoughness(roughness);
            SetMetalness(metalness);
        }

        void SetDiffuse(const std::string_view diffuse)
        {
            file_diffuse_ = diffuse;
            diffuse_      = std::make_shared<Texture>(diffuse);
        }

        void SetNormal(const std::string_view normal)
        {
            file_normal_ = normal;
            normal_      = std::make_shared<Texture>(normal);
        }

        void SetRoughness(const std::string_view roughness)
        {
            file_roughness_ = roughness;
            roughness_      = std::make_shared<Texture>(roughness);
        }

        void SetMetalness(const std::string_view metalness)
        {
            file_metalness_ = metalness;
            metalness_      = std::make_shared<Texture>(metalness);
        }

    private:
        std::string file_diffuse_{};
        std::string file_normal_{};
        std::string file_roughness_{};
        std::string file_metalness_{};
        std::string file_AO_{};
        std::string file_specular_{};

        std::shared_ptr<Texture> diffuse_   = nullptr;
        std::shared_ptr<Texture> normal_    = nullptr;
        std::shared_ptr<Texture> roughness_ = nullptr;
        std::shared_ptr<Texture> metalness_ = nullptr;
        std::shared_ptr<Texture> AO_        = nullptr;
        std::shared_ptr<Texture> specular_  = nullptr;
    };

    void SetMaterial(int index, Material material);
    void ResetMaterial(int index);
    void ResetMaterialAll();

    //! カスタムシェーダーを設定
    void SetShader(ShaderVs* vs = nullptr, ShaderPs* ps = nullptr)
    {
        overrided_shader_vs_ = vs;
        overrided_shader_ps_ = ps;
    }

    //! @brief
    //! @param meshes 描画するメッシュ番号を列挙
    //! @detail {0, 3 } 0,3を表示、1, 2 を非表示
    //!			{-1} だとすべて表示 {} だとすべて非表示
    void SetDrawMeshIndex(const std::vector<int>& meshes);

private:
    std::unordered_map<int, Material> materials_;
    std::vector<int>                  draw_meshes_{-1};
    ShaderVs*                         overrided_shader_vs_ = nullptr;    //!< 上書きする頂点シェーダー
    ShaderPs*                         overrided_shader_ps_ = nullptr;    //!< 上書きするピクセルシェーダー

    //@}

public:
    //---------------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //---------------------------------------------------------------------------
    //@{
    matrix& Matrix() override { return model_transform_; }    //!< マトリクス取得

    const matrix& GetMatrix() const override { return model_transform_; }    //!< マトリクス取得

    ComponentModelPtr SharedThis() { return std::dynamic_pointer_cast<ComponentModel>(shared_from_this()); }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override;

    //@}

private:
    //! モデル用のトランスフォーム
    matrix model_transform_ = matrix::scale(0.1f);

    Status<ModelBit>       model_status_;       //!< 状態
    std::string            path_  = "";         //!< 読み込みモデル名
    std::unique_ptr<Model> model_ = nullptr;    //!< モデルクラス

    ImGuizmo::OPERATION gizmo_operation_ = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE      gizmo_mode_      = ImGuizmo::LOCAL;

    std::vector<std::string_view> nodes_name_;
    int                           select_node_index_ = 0;
    bool                          node_manipulate_   = false;

    //! @brief アニメーション
    std::unique_ptr<Animation> animation_;
    std::string                current_animation_name_;
    std::string                old_animation_name_;

    std::unordered_map<std::string, std::shared_ptr<AnimationModifier>> animation_modifier_;    //!< EX用アニメーション

    float animation_time_ = 0.0f;
    bool  anim_loop_      = false;    //!< アニメーションループ設定

    std::vector<Animation::Desc> animations_desc_;

    //--------------------------------------------------------------------
    //! @name アタッチ処理
    //--------------------------------------------------------------------
    //@{
    matrix GetAttachMatrix() const;

    std::string           target_model_name_{};
    ComponentModelWeakPtr target_model_;

    int         object_node_index_ = -1;
    std::string object_node_name_{};

    bool use_model_node_scale_ = false;

    bool            editor_attach_node_manipulate_ = false;
    int             editor_attach_model_index_     = -1;
    ComponentModel* editor_attach_model_           = nullptr;
    int             editor_attach_node_index_      = -1;
    //@}

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_),
            cereal::make_nvp("model_transform", model_transform_),
            cereal::make_nvp("path", path_),
            cereal::make_nvp("desc", animations_desc_),
            cereal::make_nvp("model_status", model_status_.get()),
            cereal::make_nvp("current_animation", current_animation_name_),
            cereal::make_nvp("animation_time", animation_time_),
            cereal::make_nvp("anim_loop", anim_loop_));

        if(ver >= 1) {
            arc(cereal::make_nvp("target_model_name", target_model_name_));
            arc(cereal::make_nvp("object_node_name", object_node_name_));
            arc(cereal::make_nvp("use_model_node_scale", use_model_node_scale_));
        }

        if(ver >= 2) {
            if(!current_animation_name_.empty()) {
                auto play_anim = IsPlaying();
                arc(cereal::make_nvp("playing_animation", play_anim));
                if(play_anim) {
                    PlayAnimation(current_animation_name_, anim_loop_, 0.0f, animation_time_);
                }
            }
        }

        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));

        if(!path_.empty()) {
            Load(path_);
            if(!animations_desc_.empty()) {
                SetAnimation(animations_desc_);
            }
        }
    }

    //@}
};

//! @brief 外部Animation::Descのセーブロード
CEREAL_SAVELOAD_OTHER(Animation::Desc, arc, other)
{
    arc(CEREAL_NVP(other.name_),               //!< アニメーション名(任意)
        CEREAL_NVP(other.file_path_),          //!< ファイルパス
        CEREAL_NVP(other.animation_index_),    //!< ファイル内のアニメーション番号
        CEREAL_NVP(other.animation_speed_)     //!< アニメーションの再生速度(default:1.0f)
    );
}
CEREAL_CLASS_VERSION(ComponentModel, 2);

CEREAL_REGISTER_TYPE(ComponentModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentModel)
