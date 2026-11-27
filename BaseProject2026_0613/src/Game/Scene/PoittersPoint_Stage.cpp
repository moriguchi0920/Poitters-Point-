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
#include "Game/Object/PoittersPoint_Wall.h"

#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentObjectController.h>
#include <Game/Component/ComponentPlayerState.h>
#include <Game/Component/ComponentCPUState.h>

#include <Game/Component/ComponentKnockBack.h>
#include <Game/Component/ComponentStateMachine.h>
#include <Game/Component/ComponentGrabbable.h>

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
    CharacterData playerdata("data/Game/Models/Player/Player.mv1",
                             {
                                 {      "idle",        "data/Game/Models/Player/Anims/Idle.mv1", 1, 1.0f}, // Idle
                                 {      "walk",        "data/Game/Models/Player/Anims/Walk.mv1", 1, 1.0f}, // Walk
                                 {     "throw",  "data/Game/Models/Player/Anims/HandsThrow.mv1", 1, 1.0f}, // Throw
                                 {"throw_fast",   "data/Game/Models/Player/Anims/ThrowFast.mv1", 1, 1.5f}, // ThrowFast
                                 {   "lift up", "data/Game/Models/Player/Anims/HandsLiftup.mv1", 1, 1.0f}, // liftup
                                 { "grab idle",    "data/Game/Models/Player/Anims/GrabIdle.mv1", 1, 1.0f}, // GrabIdle
                                 { "grab walk",    "data/Game/Models/Player/Anims/GrabWalk.mv1", 1, 1.0f}, // GrabWalk
                                 {      "jump", "data/Game/Models/Player/Anims/Player_Jump.mv1", 1, 1.0f}, // Jump
                                 { "grab jump", "data/Game/Models/Player/Anims/Player_Jump.mv1", 1, 1.0f}, // GrabJump
    },
                             {0.11f, 0.11f, 0.11f},
                             0.3f,
                             1.0f,
                             1.0f);
    auto          player = Scene::Object::Create<Player>();
    player->SetCharacterStatus(playerdata);
    // プレイヤー入力用のステートマシンをここで付与する(Player::Initでは付与しない)
    player->AddComponent<ComponentPlayerState>();

    //Scene::Object::Create<Player>();
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
    //for(int i = 0; i < MAX_ENEMIES; i++) {
    //    Scene::Object::Create<Enemy>();
    //}

    // CPUキャラクター(操作プレイヤーと合わせて4人対戦)
    createCPU({40.0f, 8.0f, -30.0f});
    createCPU({-45.0f, 8.0f, 25.0f});
    createCPU({10.0f, 8.0f, 55.0f});

    //for(int i = 0; i < 5; i++) {
    //    Scene::Object::Create<Bullet>();
    //}
    Scene::Object::Create<Wall>();

    Scene::Object::Create<Rock>();
    Scene::Object::Create<Log>();

    Scene::Object::Create<Slime>();
    return true;
}

//! @brief 更新
void PoittersPoint_Stage::Update()
{
    //---------------------------------------------------------------
    // デバッグ操作(カメラ確認用)
    //  F7 : CPUを追加 / F8 : CPUを1体脱落させる / F9 : カメラの図形・中心点表示切替
    //---------------------------------------------------------------
    if(Input::IsKeyDown(KEY_INPUT_F7)) {
        auto  randomRange = [](float start, float end) { return start + (end - start) * ((float)rand() / RAND_MAX); };
        float theta       = randomRange(0.0f, DX_TWO_PI_F);
        float r           = randomRange(10.0f, 90.0f);
        createCPU({cosf(theta) * r, 30.0f, sinf(theta) * r});
    }
    if(Input::IsKeyDown(KEY_INPUT_F8)) {
        dropOutCPU();
    }
    if(Input::IsKeyDown(KEY_INPUT_F9)) {
        if(auto camera = Scene::Object::Get<Camera>()) {
            camera->SetDebugDraw(!camera->IsDebugDraw());
        }
    }

    //// 毎フレーム動作する
    //counter2++;

    ////テスト用
    //if(auto obj = Scene::Object::Get<Object>("OBJ")) {
    //    obj->AddTranslate({0.001, 0, 0});
    //}

    //printfDx("\nDEAD ENEMY: %d", enemy_dead_count);

    //{
    //    // Enemyという名前がついたObjectをVectorで複数取得
    //    auto enemies = Scene::Base::GetObjectsPtr<Object>("Enemy");

    //    // Vectorのメソッドでサイズを取得
    //    auto enemy_num = enemies.size();

    //    // エネミーの上限数から先ほど取得したサイズを引いてリリース済みのエネミー数を求める
    //    auto released_enemy_num = MAX_ENEMIES - enemy_num;
    //    // 死亡カウントの中身に代入する
    //    enemy_dead_count = released_enemy_num;

    //    // もし死亡カウントがエネミーの上限数以上なら
    //    if(MAX_ENEMIES <= released_enemy_num) {
    //        //Scene::Change(Scene::GetScene<TutorialX_GameOver>());

    //        bool canCreateEnemy = true;
    //        auto objs           = Scene::Object::GetArray<Enemy>();
    //        for(int i = 0; i < objs.size(); i++) {
    //            if(objs[i]->GetName() == "Enemy" || objs[i]->is_dead == false) {
    //                canCreateEnemy = false;
    //            }
    //        }

    //        if(canCreateEnemy) {
    //            // エネミーを生成する
    //            createEnemy();

    //            for(int i = 0; i < objs.size(); i++) {
    //                Scene::Object::Release(objs[i]);
    //            }
    //        }
    //    }
    //}
}

//! @brief GUI表示
void PoittersPoint_Stage::GUI()
{
    // 入力(Int)で、privateにある counter を指定
    ImGui::InputInt("Counter", &counter);

    ImGui::InputInt("(Test)Counter2", &counter2);
    ImGui::InputInt("(Test)Counter3", &counter3);

    // カメラ確認用のデバッグ操作
    ImGui::Separator();
    ImGui::Text(u8"F7:CPU追加 / F8:CPU脱落 / F9:カメラ図形表示");
    if(ImGui::Button(u8"CPUを追加")) {
        createCPU({0.0f, 30.0f, -40.0f});
    }
    ImGui::SameLine();
    if(ImGui::Button(u8"CPUを1体脱落")) {
        dropOutCPU();
    }
}

void PoittersPoint_Stage::createCPU(const float3& position)
{
    CharacterData cpu_data("data/Game/Models/Player/Player.mv1",
                           {
                               {      "idle",        "data/Game/Models/Player/Anims/Idle.mv1", 1, 1.0f}, // Idle
                               {      "walk",        "data/Game/Models/Player/Anims/Walk.mv1", 1, 1.0f}, // Walk
                               {     "throw",  "data/Game/Models/Player/Anims/HandsThrow.mv1", 1, 1.0f}, // Throw
                               {"throw_fast",   "data/Game/Models/Player/Anims/ThrowFast.mv1", 1, 1.5f}, // ThrowFast
                               {   "lift up", "data/Game/Models/Player/Anims/HandsLiftup.mv1", 1, 1.0f}, // liftup
                               { "grab idle",    "data/Game/Models/Player/Anims/GrabIdle.mv1", 1, 1.0f}, // GrabIdle
                               { "grab walk",    "data/Game/Models/Player/Anims/GrabWalk.mv1", 1, 1.0f}, // GrabWalk
                               {      "jump", "data/Game/Models/Player/Anims/Player_Jump.mv1", 1, 1.0f}, // Jump
                               { "grab jump", "data/Game/Models/Player/Anims/Player_Jump.mv1", 1, 1.0f}, // GrabJump
    },
                           {0.11f, 0.11f, 0.11f},
                           0.3f,
                           1.0f,
                           1.0f);

    cpu_count_++;
    auto cpu_character = Scene::Object::Create<Player>();
    cpu_character->SetName("CPU" + std::to_string(cpu_count_));
    cpu_character->SetCharacterStatus(cpu_data);
    cpu_character->SetTranslate(position);
    // CPU用のステートマシンをここで付与する(Player::Initでは付与しない)
    cpu_character->AddComponent<ComponentCPUState>();
}

void PoittersPoint_Stage::dropOutCPU()
{
    // 最後に生成されたCPUを場から取り除く(脱落の再現)
    // ※キャラクターはJoltのキャラクターコントローラーで動くため、SetTranslateでの瞬間移動では地面を抜けられない
    // ※物を持ち上げ中/保持中のCPUを消すと持っている物が宙に残るため、その間は対象外にする
    auto players = Scene::Object::GetArray<Player>();
    for(auto it = players.rbegin(); it != players.rend(); ++it) {
        auto& p = *it;
        if(!p || p->GetName().rfind("CPU", 0) != 0) {
            continue;
        }
        if(auto state_machine = p->GetComponent<ComponentStateMachine>()) {
            if(state_machine->GetGrabbing()) {
                continue;
            }
        }
        if(auto grabbable = p->GetComponent<ComponentGrabbable>()) {
            if(grabbable->GetIsGrabbed()) {
                continue;    // 誰かに持たれている
            }
        }
        Scene::Object::Release(p);
        break;
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
