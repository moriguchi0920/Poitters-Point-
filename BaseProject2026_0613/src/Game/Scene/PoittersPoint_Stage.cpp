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
#include "Game/System/PoittersPoint_Timer.h"
#include "Game/Object/PoittersPoint_Slime.h"
#include "Game/Object/PoittersPoint_Log.h"
#include "Game/System/PoittersPoint_PhysicsHelper.h"

#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentObjectController.h>
#include <Game/Component/ComponentPlayerState.h>
#include <Game/Component/ComponentCPUState.h>

#include <Game/Component/ComponentStateMachine.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentHitPoints.h>
#include <Game/Component/ComponentItemSpawner.h>

#include <algorithm>

namespace PoittersPoint {

// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化済み
bool PoittersPoint_Stage::Init()
{
    // 最初に1回動作する
    // ただし trueを返さなければ Initに何回も来る仕様。

    // create<>(名前、transformがいるか、更新の優先順位、描画の優先順位);

    //---------------------------------------------------------------
    // 試合の初期化
    // ※タイトル → キャラセレ → ステージ と2周目以降に来た時のため、ここで必ず戻す
    //---------------------------------------------------------------
    game_state_   = GameState::Playing;
    is_win_       = false;
    result_timer_ = 0.0f;
    cpu_count_    = 0;
    timer_.SetMaxTime(MATCH_TIME);
    timer_.Init();    // 残り時間を制限時間にする

#if 1
    // 地面(物理の静的メッシュ + 外周の壁)
    ground_ = Scene::Object::Create<Ground>();
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
    // 勝敗判定のために操作プレイヤーを覚えておく
    player_ = player;

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
    // 外周の壁は Ground が物理(Jolt)の静的ボディとして作る
    // (以前の Wall は SetTranslate で押し戻す方式で、物理で動くキャラクターには効かなかったため使わない)

    Scene::Object::Create<Rock>();
    Scene::Object::Create<Log>();

    Scene::Object::Create<Slime>();

    // --------------------------------------------------
    // アイテムスポナー(ドラゴン)の生成
    //  ステージ上のマスを巡回して投げ物を補充する
    // --------------------------------------------------
    {
        auto spawner_obj = Scene::Object::Create<Object>("ItemSpawner");
        // スポナーを置く位置(Init で最初に向かうマスの上空へ移動する)
        spawner_obj->SetTranslate({0.0f, 10.0f, 0.0f});
        // のちにモデルの変更をする際に使います
        // spawner_obj->AddComponent<ComponentModel>("data/Game/Models/UFO/.mv1");

        // アイテムスポナーコンポーネントを追加
        spawner_obj->AddComponent<ComponentItemSpawner>();
    }
    return true;
}

//! @brief 更新
void PoittersPoint_Stage::Update()
{
    // ステージ外へ落ちた物・キャラクターを中央へ戻す
    RespawnFallenObjects();

    //---------------------------------------------------------------
    // デバッグ操作
    //  F3 : 操作プレイヤーを倒す / F7 : CPUを追加 / F8 : CPUを1体倒す / F9 : カメラの図形・中心点表示切替
    //---------------------------------------------------------------
    if(Input::IsKeyDown(KEY_INPUT_F3)) {
        defeatPlayer();
    }
    if(Input::IsKeyDown(KEY_INPUT_F7)) {
        auto  randomRange = [](float start, float end) { return start + (end - start) * ((float)rand() / RAND_MAX); };
        float theta       = randomRange(0.0f, DX_TWO_PI_F);
        float r           = randomRange(10.0f, 90.0f);
        createCPU({cosf(theta) * r, 30.0f, sinf(theta) * r});
    }
    if(Input::IsKeyDown(KEY_INPUT_F8)) {
        defeatCPU();
    }
    if(Input::IsKeyDown(KEY_INPUT_F9)) {
        if(auto camera = Scene::Object::Get<Camera>()) {
            camera->SetDebugDraw(!camera->IsDebugDraw());
        }
    }

    //---------------------------------------------------------------
    // 試合の進行
    //---------------------------------------------------------------
    switch(game_state_) {
    case GameState::Playing:
        // 残り時間を減らし、勝敗を判定する
        timer_.Update();
        updateJudge();
        break;

    case GameState::Result:
        // 倒れる演出や勝敗の表示を見せてからシーンを切り替える
        result_timer_ += GetDeltaTime();
        if(RESULT_WAIT_TIME <= result_timer_) {
            if(is_win_) {
                Scene::Change(Scene::GetScene<PoittersPoint_Clear>());
            }
            else {
                Scene::Change(Scene::GetScene<PoittersPoint_GameOver>());
            }
            game_state_ = GameState::Finished;    // シーン切り替えの要求は1回だけ
        }
        break;

    case GameState::Finished:
        break;
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
    // 入力(Int)で、privateにある counter を指定
    ImGui::InputInt("Counter", &counter);

    ImGui::InputInt("(Test)Counter2", &counter2);
    ImGui::InputInt("(Test)Counter3", &counter3);

    // 試合の状態
    ImGui::Separator();
    ImGui::Text(u8"残り時間: %.1f 秒 / 生存者: %d 人", timer_.GetTimer(), countAlive());

    // デバッグ操作
    ImGui::Text(u8"F3:自分を倒す / F7:CPU追加 / F8:CPUを倒す / F9:カメラ図形表示");
    if(ImGui::Button(u8"CPUを追加")) {
        createCPU({0.0f, 30.0f, -40.0f});
    }
    ImGui::SameLine();
    if(ImGui::Button(u8"CPUを1体倒す")) {
        defeatCPU();
    }
    ImGui::SameLine();
    if(ImGui::Button(u8"自分を倒す")) {
        defeatPlayer();
    }
    if(ImGui::Button(u8"残り時間を5秒にする")) {
        timer_.SetTime(5.0f);
    }
}

//! @brief 勝敗の判定
//!
//! ルール
//!  - 操作プレイヤーが脱落した                     → 負け
//!  - 操作プレイヤー以外が全員脱落した(1人残った)  → 勝ち
//!  - 時間切れ: 生存者の中で操作プレイヤーのHPが一番多ければ勝ち(同じHPなら勝ち)、少なければ負け
void PoittersPoint_Stage::updateJudge()
{
    auto player = std::dynamic_pointer_cast<Player>(player_.lock());

    bool decided = false;

    if(!player || !player->IsAlive()) {
        // 自分が脱落
        is_win_ = false;
        decided = true;
    }
    else if(countAlive() <= 1) {
        // 自分だけが生き残った
        is_win_ = true;
        decided = true;
    }
    else if(timer_.IsTimeUp()) {
        // 時間切れ: 生存者のHPで比べる
        float my_hp    = 0.0f;
        float other_hp = 0.0f;    // 自分以外の生存者の最大HP
        for(auto& character : Scene::Object::GetArray<Player>()) {
            if(!character->IsAlive()) {
                continue;
            }
            float hp = 0.0f;
            if(auto hp_cmp = character->GetComponent<ComponentHitPoints>()) {
                hp = hp_cmp->GetHitPoints();
            }
            if(character == player) {
                my_hp = hp;
            }
            else {
                other_hp = std::max(other_hp, hp);
            }
        }
        is_win_ = (other_hp <= my_hp);
        decided = true;
    }

    if(decided) {
        game_state_   = GameState::Result;
        result_timer_ = 0.0f;
        timer_.SetPause(true);
    }
}

//! @brief 生存しているキャラクターの数を数える
int PoittersPoint_Stage::countAlive() const
{
    int count = 0;
    for(auto& character : Scene::Object::GetArray<Player>()) {
        if(character->IsAlive()) {
            count++;
        }
    }
    return count;
}

//! @brief 描画(UI)
void PoittersPoint_Stage::LateDraw()
{
    // 画面サイズが1280x720以外でも同じ見た目になるように補正
    float ADJUST_X = (WINDOW_W / 1280.0f);
    float ADJUST_Y = (WINDOW_H / 720.0f);

    const unsigned int white  = GetColor(255, 255, 255);
    const unsigned int shadow = GetColor(0, 0, 0);

    // 影付きの文字を描く(背景が明るくても読めるように)
    auto drawText = [&](float x, float y, unsigned int color, const char* text) {
        DrawString(static_cast<int>(x * ADJUST_X) + 2, static_cast<int>(y * ADJUST_Y) + 2, text, shadow);
        DrawString(static_cast<int>(x * ADJUST_X), static_cast<int>(y * ADJUST_Y), text, color);
    };

    char buf[64];

    //---------------------------------------------------------------
    // 残り時間(画面上中央) 例: 1:58
    //---------------------------------------------------------------
    SetFontSize(static_cast<int>(48 * ADJUST_X));
    int remain = static_cast<int>(ceilf(timer_.GetTimer()));
    snprintf(buf, sizeof(buf), "%d:%02d", remain / 60, remain % 60);
    // 残り10秒を切ったら赤くする
    drawText(590.0f, 20.0f, (remain <= 10) ? GetColor(255, 80, 80) : white, buf);

    //---------------------------------------------------------------
    // 残り人数・自分のHP(画面左上)
    // ※HPの画像表示は別ブランチ(HPView)で作業中のため、ここでは文字で仮表示
    // ※日本語はソースの文字コード(UTF-8)とDxLibの文字コードが合わず文字化けするため英字で表示
    //---------------------------------------------------------------
    SetFontSize(static_cast<int>(28 * ADJUST_X));
    snprintf(buf, sizeof(buf), "ALIVE %d", countAlive());
    drawText(30.0f, 20.0f, white, buf);

    if(auto player = player_.lock()) {
        if(auto hp = player->GetComponent<ComponentHitPoints>()) {
            snprintf(buf, sizeof(buf), "HP %d / %d", static_cast<int>(ceilf(hp->GetHitPoints())), static_cast<int>(hp->GetMaxHitPoints()));
            drawText(30.0f, 60.0f, white, buf);
        }
    }

    //---------------------------------------------------------------
    // 勝敗(画面中央)
    //---------------------------------------------------------------
    if(game_state_ != GameState::Playing) {
        SetFontSize(static_cast<int>(96 * ADJUST_X));
        if(is_win_) {
            drawText(470.0f, 300.0f, GetColor(255, 220, 60), "WIN!");
        }
        else {
            drawText(420.0f, 300.0f, GetColor(120, 160, 255), "LOSE...");
        }
    }

    // 他の描画に影響しないようにフォントサイズを戻す
    SetFontSize(16);
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
    // 物理ボディを持つので SetTranslate ではなく Warp で位置を変える
    PhysicsHelper::Warp(cpu_character.get(), position);
    // CPU用のステートマシンをここで付与する(Player::Initでは付与しない)
    cpu_character->AddComponent<ComponentCPUState>();
}

void PoittersPoint_Stage::defeatCPU()
{
    // 生存している最後のCPUのHPを0にする
    // 脱落処理(倒れる演出・持ち物を手放す等)は通常の被弾と同じくPlayer側で行われる
    auto players = Scene::Object::GetArray<Player>();
    for(auto it = players.rbegin(); it != players.rend(); ++it) {
        auto& p = *it;
        if(!p || !p->IsAlive() || !p->GetComponent<ComponentCPUState>()) {
            continue;
        }
        if(auto hp = p->GetComponent<ComponentHitPoints>()) {
            if(hp->IsDead()) {
                continue;    // 既にHP0で脱落待ち
            }
            hp->TakeDamage(hp->GetMaxHitPoints());
            break;
        }
    }
}

void PoittersPoint_Stage::defeatPlayer()
{
    if(auto player = player_.lock()) {
        if(auto hp = player->GetComponent<ComponentHitPoints>()) {
            hp->TakeDamage(hp->GetMaxHitPoints());
        }
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
