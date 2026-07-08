#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Utils/IniFileLib.h>

namespace Tutorial {
//-------------------------------------------------------
class Tutorial_09 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_09, u8"(9)Tutorial Componentの作り方");

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新
    //! @param delta 更新周期
    void Update() override;

    //! @brief 描画
    void Draw() override;

    //! @brief GUI
    void GUI() override;

    //! @brief 終了
    void Exit() override;

private:
};

//------------------------------------------------------------------

//! ■コンポーネントの作成方法
//! 1. Component○○○○という名前をつけ、Componentから継承させます
//! 2. BP_COMPONENT_DECL( Component○○○○, u8"ここに処理機能を書いておく" ); というものをクラス内のpublicに書きます
//! 3. Init(), Update(), Draw()などにその機能を追加します
//! 4. セーブ/ロードを可能にするには、
//!    CEREAL_SAVELOAD()、CEREAL_REGISTER_TYPE()、CEREAL_REGISTER_POLYMORPHIC_RELATION()
//!    を追加する必要があります(セーブロードが必要なければ書かなくても良い)
//! 5. あとは、オブジェクト内で、AddComponent<Component○○○○>() で追加するだけです
//! ※使用するにはインクルードをする必要があります
//!   GameフォルダにComponentを作りそこで用意するのが良いです
//! #include <Game/Component/Component○○○○.h> などとします
class ComponentSamplePlayerController : public Component
{
public:
    BP_COMPONENT_DECL(ComponentSamplePlayerController, u8"プレイヤー移動サンプルコンポーネント");

    void Init() override { __super::Init(); }

    void Update() override
    {
        __super::Update();

        // オーナー(自分がAddComponentされたObject)を取得します
        // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
        auto owner = GetOwner();

        // オーナーのMatrix( 位置、回転情報、スケール )を取得しておきます
        matrix mat = owner->GetMatrix();

        // カレントのカメラ(現在映しているカメラ)を取得する
        // カメラが存在すればカメラの向きに合わせて移動する (先のmatを差し替えている)
        if(auto camera = Scene::GetCurrentCamera().lock()) {
            auto vec = camera->GetTarget() - camera->GetPosition();
            vec.y    = 0.0f;    //< y軸は考慮しない
            mat      = HelperLib::Math::CreateMatrixByFrontVector(vec);
        }

        if(IsKey(KEY_INPUT_UP))
            owner->AddTranslate(mat.axisZ());

        if(IsKey(KEY_INPUT_DOWN))
            owner->AddTranslate(-mat.axisZ());

        if(IsKey(KEY_INPUT_RIGHT))
            owner->AddTranslate(mat.axisX());

        if(IsKey(KEY_INPUT_LEFT))
            owner->AddTranslate(-mat.axisX());
    }

    void GUI() override
    {
        __super::GUI();

        ImGui::Begin(GetOwner()->GetName().data());
        {
            ImGui::Separator();
            if(ImGui::TreeNode(u8"SamplePlayerController")) {
                // GUI上でオーナーから自分(SamplePlayerController)を削除します
                if(ImGui::Button(u8"削除"))
                    GetOwner()->RemoveComponent(shared_from_this());

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

//--------------------------------------------------------------------------

bool Tutorial_09::Init()
{
    // とりあえずカメラを用意する
    Scene::Object::Create<Object>()                         // カメラオブジェクト
        ->SetName("Camera")                                 // 名前設定
        ->AddComponent<ComponentCamera>()                   // カメラコンポーネント
        ->SetPositionAndTarget({0, 35, 80}, {0, 20, 0});    // ポジションと注視点

    // オブジェクトを作成して利用するコンポーネントを張り付けるだけです
    // コンポーネント化するとほかのObjectでも利用することができます
    auto obj = Scene::Object::Create<Object>()->SetName(u8"テストObject");
    obj->AddComponent<ComponentSamplePlayerController>();
    obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1")    // モデル
        ->SetScaleAxisXYZ({0.08f});                                      //大きさ設定

    return true;
}

void Tutorial_09::Update()
{
}

void Tutorial_09::Draw()
{
}

void Tutorial_09::GUI()
{
    __super::GUI();

    ImGui::TextColored({1, 1, 0, 1}, u8"[ComponentSamplePlayerController]");
    ImGui::TextColored({0, 1, 0, 1}, u8"AddComponentするとカメラに対してカーソル移動できます");
    ImGui::Spacing();
    ImGui::Separator();
}

void Tutorial_09::Exit()
{
}

}    // namespace Tutorial

CEREAL_REGISTER_TYPE(Tutorial::ComponentSamplePlayerController)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Tutorial::ComponentSamplePlayerController, Component)
