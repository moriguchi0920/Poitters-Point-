//---------------------------------------------------------------------------
//! @file   PoittersPoint_Stage.cpp
//! @brief  PoittersPointステージシーン
//---------------------------------------------------------------------------
#include "PoittersPoint_Stage.h"
#include "PoittersPoint_GameOver.h"
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
    PoittersPoint_Timer::Init();
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
    PoittersPoint_Timer::Update();
    // 毎フレーム動作する
    counter2++;

    //テスト用
    if(auto obj = Scene::Object::Get<Object>("OBJ")) {
        obj->AddTranslate({0.001, 0, 0});
    }

    printfDx("\n DEAD ENEMY: %d", enemy_dead_count);

    printfDx("\n TIME : %.2f", PoittersPoint_Timer::GetTimer());

    {
        // Enemyという名前がついたObjectをVectorで複数取得
        auto enemies = Scene::Base::GetObjectsPtr<Object>("Enemy");

        // Vectorのメソッドでサイズを取得
        auto enemy_num = enemies.size();

        // エネミーの上限数から先ほど取得したサイズを引いてリリース済みのエネミー数を求める
        auto released_enemy_num = MAX_ENEMIES - enemy_num;
        // 死亡カウントの中身に代入する
        enemy_dead_count = released_enemy_num;

        // もし死亡カウントがエネミーの上限数以上なら
        if(MAX_ENEMIES <= released_enemy_num) {
            //Scene::Change(Scene::GetScene<TutorialX_GameOver>());

            bool canCreateEnemy = true;
            auto objs           = Scene::Object::GetArray<Enemy>();
            for(int i = 0; i < objs.size(); i++) {
                if(objs[i]->GetName() == "Enemy" || objs[i]->is_dead == false) {
                    canCreateEnemy = false;
                }
            }

            if(canCreateEnemy) {
                // エネミーを生成する
                createEnemy();

                for(int i = 0; i < objs.size(); i++) {
                    Scene::Object::Release(objs[i]);
                }
            }
        }
    }
}

//! @brief GUI表示
void PoittersPoint_Stage::GUI()
{
    // 入力(Int)で、privateにある counter を指定
    ImGui::InputInt("Counter", &counter);

    ImGui::InputInt("(Test)Counter2", &counter2);
    ImGui::InputInt("(Test)Counter3", &counter3);
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
