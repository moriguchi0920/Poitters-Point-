#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentSequencer.h>

namespace Tutorial {
//-------------------------------------------------------
class Tutorial_10 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_10, u8"(10)Tutorial Sequencerの使い方");

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override
    {
        //-------------------------------------------------------------------------------
        // カメラ
        //-------------------------------------------------------------------------------
        Scene::Object::Create<Object>()                          //< Object作成
            ->SetName(u8"標準カメラ")                            //< 名前設定
            ->AddComponent<ComponentCamera>()                    //< 標準カメラ
            ->SetPerspective(45)                                 //< 画角
            ->SetPositionAndTarget({0, 15, -50}, {0, 10, 0});    //< カメラ位置と見るところ

        //-------------------------------------------------------------------------------
        // 地面
        //-------------------------------------------------------------------------------
        auto ground = Scene::Object::Create<Object>()    //< Object作成
                          ->SetName(u8"地面");           //< 名前設定

        ground->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
        ground->AddComponent<ComponentCollisionModel>()->AttachToModel();

        //-------------------------------------------------------------------------------
        // Mouse
        //-------------------------------------------------------------------------------
        auto obj = Scene::Object::Create<Object>()    //< Object作成
                       ->SetName(u8"Mouse")           //< 名前設定
                       ->SetTranslate({0, 0, 0});     //< 位置設定
        // モデル
        obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1")
            ->SetAnimation({
                {"idle", "data/Sample/Player/Anim/Idle2.mv1", 0, 1.0f}, // Idle
                {"jump", "data/Sample/Player/Anim/Jump2.mv1", 0, 1.0f}  // Jump
        });

        // コリジョン
        obj->AddComponent<ComponentCollisionCapsule>()    //
            ->SetRadius(3.0f)                             //
            ->SetHeight(12.5f)
            ->UseGravity();

        //-------------------------------------------------------------------------------
        // Boss
        //-------------------------------------------------------------------------------
        auto boss = Scene::Object::Create<Object>()    //< Object作成
                        ->SetName(u8"Boss")            //< 名前設定
                        ->SetTranslate({3, 0, 0});     //< 位置設定
        // モデル
        boss->AddComponent<ComponentModel>("data/Sample/Boss/model.mv1")
            ->SetAnimation({
                {"idle",  "data/Sample/Player/Anim/Idle.mv1", 0, 1.0f}, // Idle
                {"idle",  "data/Sample/Player/Anim/Walk.mv1", 0, 1.0f}, // Walk
                {"jump", "data/Sample/Player/Anim/Death.mv1", 0, 1.0f}  // Deah
        });

        // コリジョン
        boss->AddComponent<ComponentCollisionCapsule>()    //
            ->SetRadius(3.5f)                              //
            ->SetHeight(22.3f)
            ->UseGravity();

        //-------------------------------------------------------------------------------
        // シーケンサー
        // ▣データをセーブする
        // S1. 以下のシーケンサなどで作成されたオブジェクトをGUIで選択して「Save」を押します
        // S2. data/_save/object/(オブジェクト名).txt　にセーブされます
        //
        // ▣データをロードする
        // L1. プログラム上で、オブジェクト->Load("オブジェクト名"); とする
        // GUI上でロードする場合は、必要ないオブジェクトの名前をロードするファイル名にし「ロード」ボタンを押します
        //
        // ※その際にロードしているオブジェクトは削除され、新たにロードされたオブジェクトが発生します
        //
        // ▣Playする
        // P1. ロードされたオブジェクトを再取得する
        // P2. ロードされたオブジェクトのComponentSequencer を Play する
        //-------------------------------------------------------------------------------
        auto sequencer = Scene::Object::Create<Object>()    //< シーケンサを作る
                             ->SetName("Sequencer");

        if(sequencer->Load("Sequencer"))    //< L1
        {
            // ロードされた場合(シーケンサは変わっているので取り直す)
            if((sequencer = Scene::Object::Get<Object>("Sequencer")))    //< P1
            {
                //再生する
                if(auto seq = sequencer->GetComponent<ComponentSequencer>())    //< P2
                    seq->Play();
            }
        }
        else {
            // ロード失敗し、ファイルがない場合は初期化してつけておく(初期はこれになります)
            sequencer->AddComponent<ComponentSequencer>();
        }

        return true;
    }

    //! @brief 更新
    //! @param delta 更新周期
    void Update() override {}

    //! @brief 描画
    void Draw() override {}

    //! @brief GUI
    void GUI() override {}

    //! @brief 終了
    void Exit() override {}

private:
    bool once_ = false;
};

}    // namespace Tutorial
