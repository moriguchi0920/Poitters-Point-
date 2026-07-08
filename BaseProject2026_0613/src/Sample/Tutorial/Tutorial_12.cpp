#include <System/Scene.h>
#include <System/Component/ComponentFilterFade.h>
#include <System/Utils/IniFileLib.h>

namespace Tutorial {
//-------------------------------------------------------
// 通常の Header部
//-------------------------------------------------------
class Tutorial_12 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_12, u8"(12)Tutorial シーン切り替え");

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新
    //! @param delta 更新周期
    void Update() override;

    //! @brief 描画
    void Draw() override;

    //! @brief 終了
    void Exit() override;

private:
};

class Tutorial_12_Next : public Scene::Base
{
public:
    // BP_CLASS_DECL(Tutorial_12_Next, u8"(12Next)Tutorial シーン切り替え");

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新
    //! @param delta 更新周期
    void Update() override;

    //! @brief 描画
    void Draw() override;

    //! @brief 終了
    void Exit() override;

private:
};

//------------------------------------------------------------------
// 通常のCPP部
//------------------------------------------------------------------
namespace {
void FadeIn()
{
    Scene::Object::Create<Object>()              // フェード用オブジェクト
        ->SetName("FadeIn")                      // 名前設定
        ->AddComponent<ComponentFilterFade>()    // フェードコンポーネント
        ->StartFadeIn();                         // フェードインスタート
}

bool WaitFadeIn()
{
    if(auto obj = Scene::Object::Get<Object>("FadeIn")) {
        auto fade = obj->GetComponent<ComponentFilterFade>();
        if(fade->IsFadeIn()) {
            if(!fade->IsFinishFade())
                return true;

            Scene::Object::Release("FadeIn");
            return false;
        }
    }

    return false;
}

void FadeOut()
{
    Scene::Object::Create<Object>()              // フェード用オブジェクト
        ->SetName("FadeOut")                     // 名前設定
        ->AddComponent<ComponentFilterFade>()    // フェードコンポーネント
        ->StartFadeOut();                        // フェードインスタート
}

bool WaitFadeOut()
{
    if(auto obj = Scene::Object::Get<Object>("FadeOut")) {
        auto fade = obj->GetComponent<ComponentFilterFade>();
        if(fade->IsFadeOut()) {
            if(!fade->IsFinishFade()) {
                return true;
            }
            Scene::Object::Release("FadeOut");
            return false;
        }
    }

    return true;
}
}    // namespace

bool Tutorial_12::Init()
{
    //
    Scene::Object::Create<Object>()                         // タイトルとかゲームのプレイヤーなど
        ->SetName("Obj-Camera")                             //
        ->AddComponent<ComponentCamera>()                   //< 標準カメラ
        ->SetPerspective(45)                                //< 画角
        ->SetPositionAndTarget({0, 5, 0}, {0, 30, 100});    //< SpringArmとコリジョンを使う場合はこれで

    FadeIn();
    return true;
}

void Tutorial_12::Update()
{
    // フェードイン中なら待つ
    if(WaitFadeIn())
        return;

    if(Input::IsKeyDown(KEY_INPUT_SPACE)) {
        // 次のシーンに移動準備します
        FadeOut();
    }

    // フェードイン中なら待つ
    if(!WaitFadeOut()) {
        // シーンを切り替えます
        Scene::Change(Scene::GetScene<Tutorial_12_Next>());
    }
}

void Tutorial_12::Draw()
{
    DrawFormatString(100, 100, GetColor(255, 255, 255), "Scene : %s", Scene::GetCurrentScene()->GetName().data());
}

void Tutorial_12::Exit()
{
}

//------------------------------------------------------------------
// 通常のCPP部
//------------------------------------------------------------------
bool Tutorial_12_Next::Init()
{
    //
    Scene::Object::Create<Object>()    // タイトルとかゲームのプレイヤーなど
        ->SetName("ObjNext");

    FadeIn();
    return true;
}

void Tutorial_12_Next::Update()
{
    // フェードイン中なら待つ
    if(WaitFadeIn())
        return;

    if(Input::IsKeyDown(KEY_INPUT_SPACE)) {
        // 次のシーンに移動準備します
        FadeOut();
    }

    // フェードイン中なら待つ
    if(!WaitFadeOut()) {
        // シーンを切り替えます
        Scene::Change(Scene::GetScene<Tutorial_12>());
    }
}

void Tutorial_12_Next::Draw()
{
    DrawFormatString(100, 100, GetColor(255, 255, 255), "Scene : %s", Scene::GetCurrentScene()->GetName().data());
}

void Tutorial_12_Next::Exit()
{
}

}    // namespace Tutorial
