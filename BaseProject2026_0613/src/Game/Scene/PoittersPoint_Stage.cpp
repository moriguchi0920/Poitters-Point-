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
#include "Game/Object/PoittersPoint_Slime.h"
#include "Game/Object/PoittersPoint_Log.h"
#include "Game/System/PoittersPoint_Timer.h"
#include "Game/System/PoittersPoint_PhysicsHelper.h"

#include <System/Scene.h>
#include <Game/Component/ComponentPlayerState.h>
#include <Game/Component/ComponentCPUState.h>
#include <Game/Component/ComponentGrabbable.h>

namespace PoittersPoint {

namespace {
//! @brief 標準キャラクターのデータ(プレイヤー/CPU共通。キャラ選択を作る時はここを差し替える)
CharacterData MakeDefaultCharacterData()
{
    return CharacterData("data/Game/Models/Player/Player.mv1",
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
}
}    // namespace

//! @brief 初期化
//! @return 初期化済み
bool PoittersPoint_Stage::Init()
{
    // 最初に1回動作する
    // ただし trueを返さなければ Initに何回も来る仕様。

    // 地面(物理の静的メッシュ + 外周の壁)
    ground_ = Scene::Object::Create<Ground>();

    // プレイヤー
    {
        auto player = Scene::Object::Create<Player>();
        player->SetCharacterStatus(MakeDefaultCharacterData());
        // プレイヤー入力用のステートマシンをここで付与する(Player::Initでは付与しない)
        player->AddComponent<ComponentPlayerState>();
    }

    Scene::Object::Create<Camera>();

    // CPU
    {
        auto cpu_character = Scene::Object::Create<Player>();
        cpu_character->SetName("CPU1");
        cpu_character->SetCharacterStatus(MakeDefaultCharacterData());
        // 物理ボディを持つので SetTranslate ではなく Warp で位置を変える
        PhysicsHelper::Warp(cpu_character.get(), {20.0f, 8.0f, 0.0f});
        // CPU用のステートマシンをここで付与する(Player::Initでは付与しない)
        cpu_character->AddComponent<ComponentCPUState>();
    }

    // 投げ物
    Scene::Object::Create<Rock>();
    Scene::Object::Create<Log>();
    Scene::Object::Create<Slime>();

    return true;
}

//! @brief 更新
void PoittersPoint_Stage::Update()
{
    RespawnFallenObjects();
}

//! @brief ステージ外へ落ちた物・キャラクターを中央へ戻す
void PoittersPoint_Stage::RespawnFallenObjects()
{
    auto ground = ground_.lock();
    if(!ground)
        return;

    const float fall_out_y = ground->GetFallOutHeight();
    for(auto& obj : Scene::Object::GetArray<Object>()) {
        // 物理で動く物(投げ物・キャラクター)だけが対象。持たれている物は持ち主が落ちれば一緒に戻る
        auto grabbable = obj->GetComponent<ComponentGrabbable>();
        if(!grabbable || grabbable->IsHeld())
            continue;

        if((float)obj->GetTranslate().y < fall_out_y)
            PhysicsHelper::Warp(obj.get(), ground->GetRespawnPosition());
    }
}

//! @brief GUI表示
void PoittersPoint_Stage::GUI()
{
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
