//---------------------------------------------------------------------------
//! @file   PoittersPoint_Stage.cpp
//! @brief  PoittersPointステージシーン
//---------------------------------------------------------------------------
#include "PoittersPoint_Stage.h"
#include "PoittersPoint_GameOver.h"
#include "PoittersPoint_Clear.h"
#include "Game/Object/PoittersPoint_Player.h"
#include "Game/Object/PoittersPoint_Ground.h"
#include "Game/Object/PoittersPoint_Camera.h"
#include "Game/Object/PoittersPoint_Enemy.h"
#include "Game/Object/PoittersPoint_Rock.h"
#include "Game/system/PoittersPoint_Timer.h"
#include "Game/Object/PoittersPoint_Slime.h"
#include "Game/Object/PoittersPoint_Log.h"

#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentObjectController.h>

namespace PoittersPoint {

// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化済み
bool PoittersPoint_Stage::Init()
{
    // 最初に1回動作する
    // ただし trueを返さなければ Initに何回も来る仕様。

    // create<>(名前、transformがいるか、更新の優先順位、描画の優先順位);

#if 1
    Scene::Object::Create<Ground>();
#else
    // 地面生成(Component: モデル、モデルコリジョン)
    {
        auto ground = Scene::Object::Create<Object>("Ground");
        Scene::Object::Create<PoittersPoint_Timer>();
        ground->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
        ground->AddComponent<ComponentCollisionModel>();
        if(auto collision = ground->GetComponent<ComponentCollisionModel>()) {
            // 所属するグループを「GROUND」とします
            collision->SetCollisionGroup(ComponentCollision::CollisionGroup::GROUND);
            collision->AttachToModel();    // コリジョンをモデルに合わせる
        }
    }
#endif
#if 1    //プレイヤー作成
    Scene::Object::Create<Player>();
#else
    // プレイヤー生成(Component: モデル、カプセルコリジョン、オブジェクトコントローラ)
    {
        auto player = Scene::Object::Create<Object>("Player");
        player->SetTranslate({0, 5, 0});
        player->AddComponent<ComponentModel>("data/Sample/Player/Model.mv1");
        if(auto model = player->GetComponent<ComponentModel>()) {
            model->SetAnimation({
                {"idle", "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f}, // Idle
                {"walk", "data/Sample/Player/Anim/Walk.mv1", 1, 1.0f}  // Walk
            });
        }
        player->AddComponent<ComponentCollisionCapsule>();
        if(auto collision = player->GetComponent<ComponentCollisionCapsule>()) {
            // 所属するグループを「PLAYER」とします
            collision->SetCollisionGroup(ComponentCollision::CollisionGroup::PLAYER);
            // Collisionの重力を有効にします
            collision->UseGravity();
            collision->SetRadius(3.0f);     // コリジョンの半径を3.0 にする
            collision->SetHeight(13.0f);    // コリジョンの高さを13.0 にする
        }
        player->AddComponent<ComponentObjectController>();
        if(auto ctl = player->GetComponent<ComponentObjectController>()) {
            ctl->SetMoveSpeed(0.3f);
            ctl->SetRotateSpeed(20.0f);
        }
    }
#endif

#if 1
    Scene::Object::Create<Camera>();
#else
    // カメラ生成(Component: モデル、カプセルコリジョン、オブジェクトコントローラ)
    {
        auto camera = Scene::Object::Create<Object>("Camera");    //オブジェクトをカメラという名前で生成
        camera->AddComponent<ComponentCamera>();                  //カメラコンポーネントを付ける
        if(auto c = camera->GetComponent<ComponentCamera>()) {
            c->SetPositionAndTarget({0, 20, -50}, {0, 10, 0});
        }
        camera->AddComponent<ComponentSpringArm>();
        if(auto c = camera->GetComponent<ComponentSpringArm>()) {
            c->SetSpringArmObject("Player");
        }
    }
#endif
    for(int i = 0; i < MAX_ENEMIES; i++) {
        Scene::Object::Create<Enemy>();
    }

    //for(int i = 0; i < 5; i++) {
    //    Scene::Object::Create<Bullet>();
    //}

    Scene::Object::Create<Rock>();
    Scene::Object::Create<Log>();

    Scene::Object::Create<Slime>();

    return true;
}

//! @brief 更新
void PoittersPoint_Stage::Update()
{
    // 毎フレーム動作する
    counter2++;

    //テスト用
    if(auto obj = Scene::Object::Get<Object>("OBJ")) {
        obj->AddTranslate({0.001, 0, 0});
    }

    // プレイヤーを取得
    auto player = Scene::Object::Get<Player>();

    // プレイヤー死亡時は GameOver へ移行せず、敵同士の戦いを継続する
    // プレイヤーは無効化のみ行う（戦闘から除外）
    bool player_dead = false;
    if(player) {
        if(player->IsDead()) {
            player_dead = true;
            player->SetStatus(Object::StatusBit::NoUpdate, true);
            player->SetStatus(Object::StatusBit::NoDraw, true);
        }
    }
    else {
        player_dead = true;
    }

    // 敵の生存状態をチェック
    auto enemies           = Scene::Object::GetArray<Enemy>();
    int  alive_enemy_count = 0;
    for(auto& e : enemies) {
        if(!e->is_dead) {
            alive_enemy_count++;
        }
    }

    // プレイヤー死亡 かつ 敵が1体 → クリア（敵の勝ち）
    if(player_dead && alive_enemy_count == 1) {
        Scene::Change(Scene::GetScene<PoittersPoint_Clear>());
        return;
    }

    // プレイヤー生存 かつ 敵全滅 → クリア（プレイヤーの勝ち）
    if(!player_dead && alive_enemy_count == 0) {
        Scene::Change(Scene::GetScene<PoittersPoint_Clear>());
        return;
    }
}

//! @brief GUI表示
void PoittersPoint_Stage::GUI()
{
    // 入力(Int)で、privateにある counter を指定
    ImGui::InputInt("Counter", &counter);

    ImGui::InputInt("(Test)Counter2", &counter2);
    ImGui::InputInt("(Test)Counter3", &counter3);

    // 敵勝利表示
    if(enemy_won_) {
        ImGui::Begin("敵の勝利");
        ImGui::Text("%sの勝利!", winner_enemy_name_.c_str());
        ImGui::End();
    }
}

void PoittersPoint_Stage::AddDeadEnemy()
{
    enemy_dead_count++;
    if(MAX_ENEMIES <= enemy_dead_count) {
        createEnemy();
        enemy_dead_count = 0;
    }
}

void PoittersPoint_Stage::createEnemy()
{
    for(int i = 0; i < MAX_ENEMIES; i++) {
        Scene::Object::Create<Enemy>();
    }
}

}    // namespace PoittersPoint
