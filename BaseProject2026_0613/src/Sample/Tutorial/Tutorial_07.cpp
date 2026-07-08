#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

// キャラクターの当たりはカプセルでつけます
#include <System/Component/ComponentCollisionCapsule.h>

// 地面には地面当たりも必要になります
#include <System/Component/ComponentCollisionModel.h>

namespace Tutorial {
//-------------------------------------------------------
class Tutorial_07 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_07, u8"(7)Tutorial キャラクタージャンプ方法");

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

//! @brief マウス
class Object07 : public Object
{
    //! @brief ジャンプステート
    enum class JumpMode
    {
        JUMP_NONE,              //!< 無し
        JUMP_START,             //!< スタート
        JUMP_ING,               //!< ジャンプ中
        JUMP_IS_TOUCHGROUND,    //!< 地面にタッチしたか
        JUMP_TOUCHEDGROUND,     //!< 地面にタッチした!
        JUMP_END,               //!< 終了アクション

        JUMP_NUM,
    };

    //! @brief ジャンプステートのモード
    JumpMode jump_mode = JumpMode::JUMP_NONE;

public:
    //! @brief 他のコリジョンと当たったとき
    //! @param hitInfo コリジョン情報
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override
    {
        __super::OnHit(hitInfo);

        // 地面にあたっている判定状態か?
        if(jump_mode == JumpMode::JUMP_IS_TOUCHGROUND) {
            // 地面にあたっている判定状態ならばモードを変更
            auto owner = hitInfo.hit_collision_->GetOwnerPtr();
            if(owner->GetNameDefault() == u8"地面") {
                //printfDx( "地面と当たった" );
                // モードを当たったことにする
                jump_mode = JumpMode::JUMP_TOUCHEDGROUND;
            }
        }
    }

    //! フットワーク状態にする
    void Idle()
    {
        // ジャンプしてない場合は、フットワーク状態にする
        if(jump_mode == JumpMode::JUMP_NONE) {
            if(auto model = GetComponent<ComponentModel>())
                model->PlayAnimation("idle");
        }
    }

    //! ジャンプ状態にする
    void Jump()
    {
        // ジャンプしてない場合は、ジャンプする
        if(jump_mode == JumpMode::JUMP_NONE) {
            if(auto model = GetComponent<ComponentModel>()) {
                model->PlayAnimation("jump");
                jump_mode = JumpMode::JUMP_START;
            }
        }
    }

    // 処理更新
    void Update() override
    {
        __super::Update();

        // モデルが設定されてないと処理しない
        auto model = GetComponent<ComponentModel>();
        if(model == nullptr)
            return;

        // ジャンプモードに合わせて処理を変更
        switch(jump_mode) {
        case JumpMode::JUMP_NONE:
            // なにもしない
            break;

        case JumpMode::JUMP_START:
            // ジャンプアクションが飛び上がれる状態になっているか?
            if(model->GetAnimationTime() >= jump_start)
                jump_mode = JumpMode::JUMP_ING;
            break;

        case JumpMode::JUMP_ING:
            // 飛び上がり中は、上に動かす
            AddTranslate({0, add_y, 0});

            // 着地モーションが出ないようにその前で止める
            if(model->GetAnimationTime() >= jump_end) {
                model->PlayPause();
                jump_mode = JumpMode::JUMP_IS_TOUCHGROUND;
            }
            break;

        case JumpMode::JUMP_IS_TOUCHGROUND:
            // 着地待ち
            AddTranslate({0, add_y, 0});
            break;

        case JumpMode::JUMP_TOUCHEDGROUND:
            // 着地したら、アニメーションをさらに動かす
            model->PlayPause(false);
            jump_mode = JumpMode::JUMP_END;

        case JumpMode::JUMP_END:
            // アニメーションがおわったか?
            if(!model->IsPlaying()) {
                // フットワークのアニメーションを出す (ブレンドを長めにしている)
                model->PlayAnimation("idle", true, 1.0f);

                // またジャンプできるようになにもしない状態に戻す
                jump_mode = JumpMode::JUMP_NONE;
            }

            break;
        }
    }

private:
    const float add_y      = 0.8f;
    const float jump_start = 0.8f;
    const float jump_end   = 1.4f;

    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Object", cereal::base_class<Object>(this))); }
};

bool Tutorial_07::Init()
{
    Scene::Object::Create<Object>()                          //< Object作成
        ->SetName(u8"標準カメラ")                            //< 名前設定
        ->AddComponent<ComponentCamera>()                    //< 標準カメラ
        ->SetPerspective(45)                                 //< 画角
        ->SetPositionAndTarget({0, 15, -50}, {0, 10, 0});    //< カメラ位置と見るところ

    // 地面を作成
    auto ground = Scene::Object::Create<Object>()    //< Object作成
                      ->SetName(u8"地面");           //< 名前設定

    ground->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
    ground->AddComponent<ComponentCollisionModel>()->AttachToModel();

    // オブジェクトを作成します
    // 名前なども任意につけましょう。(位置も設定しています)
    auto obj = Scene::Object::Create<Object07>()    //< Object作成
                   ->SetName(u8"オブジェクト")      //< 名前設定
                   ->SetTranslate({0, 0, 0});       //< 位置設定

    // 作成したオブジェクトにモデルをつけます
    // モデルを取得しておきます (変数をクラス内に持つ必要はありません)
    // Drawで描画する必要はなく、自動的にComponentModelが描画します
    auto model = obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1");

    // 追加した「モデル」にアニメーションを準備することでアクションを可能にします。
    // { アニメーションタグ(自由につけれます), ファイル名, 番号, スピード }
    // 番号は「tools/DxLibModelViewer_64bit.exe」にて番号を確認する必要があります
    model->SetAnimation({
        {"idle", "data/Sample/Player/Anim/Idle2.mv1", 0, 1.0f}, // Idle
        {"jump", "data/Sample/Player/Anim/Jump2.mv1", 0, 1.0f}  // Jump
    });

    // 次のコマンドで実際にアニメーションを行います
    model->PlayAnimation("idle", true);

    // オブジェクトに当たりと重力をつける
    obj->AddComponent<ComponentCollisionCapsule>()    //カプセル当たりを追加
        ->SetRadius(3)
        ->SetHeight(10)
        ->UseGravity();

    return true;
}

void Tutorial_07::Update()
{
    // オブジェクトがいなければ何もしない
    if(auto obj = Scene::Object::Get<Object07>(u8"オブジェクト")) {
        if(auto model = obj->GetComponent<ComponentModel>()) {
            if(IsKey(KEY_INPUT_1)) {
                // フットワークはループ設定にする
                obj->Idle();
            }

            if(IsKey(KEY_INPUT_2)) {
                obj->Jump();
            }
        }
    }
}

void Tutorial_07::Draw()
{
    DrawFormatString(0, 0, GetColor(0, 255, 255), "ComponentModelテスト\n");
}

void Tutorial_07::GUI()
{
    // HowToUseというGUIWindowを以下の内容で表示する
    ImGui::Begin("HowToUse");
    {
        ImGui::Separator();
        ImGui::Text("ジャンプ作成方法　(ComponentCollisionを使用)\n "    //
                    u8"\n"                                               //
                    u8"浮き上がるタイミングはかって上に飛びあげます\n"
                    u8"コリジョンの重力に任せて落とすと楽にジャンプできます\n"
                    u8"ここでは行っていませんが、\n"
                    u8"ジャンプ中は着地アクションが出ないようにポーズをかけることや\n"
                    u8"アクション自体を分けて作成することも多く\n"
                    u8"着地になったら着地モーションを出すのが基本となります\n"
                    u8"\n"
                    u8"今回使用しているモーションは少し飛び上がっていますが\n"
                    u8"ソフト的に動作をさせることで飛び上がってないモーションでも\n"
                    u8"ジャンプさせることが可能です\n"
                    u8"\n"
                    u8"キー「1」 : フットワークモーション\n"
                    u8"キー「2」 : ジャンプモーション\n");
    }
    ImGui::End();
}

void Tutorial_07::Exit()
{
}

}    // namespace Tutorial

CEREAL_REGISTER_TYPE(Tutorial::Object07)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Tutorial::Object07, Object)
