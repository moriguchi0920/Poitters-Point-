#include <System/Scene.h>

// モデルコンポーネントを使用するためには以下のインクルードが必要になります
#include <System/Component/ComponentModel.h>

namespace Tutorial {
//-------------------------------------------------------
class Tutorial_06 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_06, u8"(6)Tutorial Modelコンポーネント使用方法");

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新
    //! @param delta 更新周期
    void Update() override;

    //! @brief 描画
    void Draw() override;

    //! @brief デバッグ表示用GUI
    void GUI() override;

    //! @brief 終了
    void Exit() override;

private:
};

//------------------------------------------------------------------

bool Tutorial_06::Init()
{
    Scene::Object::Create<Object>()                          //< Object作成
        ->SetName(u8"標準カメラ")                            //< 名前設定
        ->AddComponent<ComponentCamera>()                    //< 標準カメラ
        ->SetPerspective(45)                                 //< 画角
        ->SetPositionAndTarget({0, 15, -50}, {0, 10, 0});    //< カメラ位置と見るところ

    // オブジェクトを作成します
    // 名前なども任意につけましょう。(位置も設定しています)
    auto obj = Scene::Object::Create<Object>()    //< Object作成
                   ->SetName(u8"オブジェクト")    //< 名前設定
                   ->SetTranslate({0, 0, 0});     //< 位置設定

    // 作成したオブジェクトにモデルをつけます
    // モデルを取得しておきます (変数をクラス内に持つ必要はありません)
    // Drawで描画する必要はなく、自動的にComponentModelが描画します
    auto model = obj->AddComponent<ComponentModel>("data/Sample/Boss/model.mv1");

    // 追加した「モデル」にアニメーションを準備することでアクションを可能にします。
    // { アニメーションタグ(自由につけれます), ファイル名, 番号, スピード }
    // 番号は「tools/DxLibModelViewer_64bit.exe」にて番号を確認する必要があります
    model->SetAnimation({
        { "idle",  "data/Sample/Boss/Anim/Idle.mv1", 0, 1.0f},
        { "walk",  "data/Sample/Boss/Anim/Walk.mv1", 0, 1.0f},
        {"death", "data/Sample/Boss/Anim/Death.mv1", 0, 1.0f}
    });

    // 次のコマンドで実際にアニメーションを行います
    model->PlayAnimation("idle");

    return true;
}

void Tutorial_06::Update()
{
    // オブジェクトがいなければ何もしない
    if(auto obj = Scene::Object::Get<Object>(u8"オブジェクト")) {
        if(auto model = obj->GetComponent<ComponentModel>()) {
            if(IsKeyDown(KEY_INPUT_1)) {
                // フットワークはループ設定にする
                model->PlayAnimation("idle", true);
            }

            if(IsKeyDown(KEY_INPUT_2)) {
                // 歩きはループ設定にする
                model->PlayAnimation("walk", true);
            }

            if(IsKeyDown(KEY_INPUT_3)) {
                // やられモーションはループ設定にしない
                model->PlayAnimation("death");
            }
        }
    }
}

void Tutorial_06::Draw()
{
    DrawFormatString(0, 0, GetColor(0, 255, 255), "ComponentModelテスト\n");
}

void Tutorial_06::GUI()
{
    // HowToUseというGUIWindowを以下の内容で表示する
    ImGui::Begin("HowToUse");
    {
        ImGui::Separator();
        ImGui::Text("Componentとは?\n "    //
                    u8"\n"                 //
                    u8"オブジェクトに追加できる能力です。\n"
                    u8"今回はComponentModel(モデル描画機能)を追加しています\n"
                    u8"オブジェクト自身のTransformに影響しますが、\n"
                    u8"モデル自身にもTransformがあります\n"
                    u8"\n"
                    u8"基本的にオブジェクトの向きにモデルのTransformを合わせて\n"
                    u8"オブジェクトで操作するのが基本になります\n"
                    u8"\n"
                    u8"キー「1」 : フットワークモーション\n"
                    u8"キー「2」 : 歩きモーション\n"
                    u8"キー「3」 : やられモーション\n");
    }
    ImGui::End();
}

void Tutorial_06::Exit()
{
}

}    // namespace Tutorial
