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

//! @brief 初期化
//! @return 初期化済み
bool PoittersPoint_Stage::Init()
{
    // 地面生成
    Scene::Object::Create<Ground>();

    // プレイヤー作成
    Scene::Object::Create<Player>();

    // カメラ生成
    Scene::Object::Create<Camera>();

    // 敵の生成
    for(int i = 0; i < MAX_ENEMIES; i++) {
        Scene::Object::Create<Enemy>();
    }

    // オブジェクトの生成
    Scene::Object::Create<Rock>();
    Scene::Object::Create<Log>();
    Scene::Object::Create<Slime>();

    return true;
}

//! @brief 更新
void PoittersPoint_Stage::Update()
{
    counter2++;

    // テスト用
    if(auto obj = Scene::Object::Get<Object>("OBJ")) {
        obj->AddTranslate({0.001f, 0.0f, 0.0f});
    }

    // ===================================================
    // ★ スライムの再生成（リスポーン）ロジック
    // ===================================================
    // フィールド上に存在するSlimeのリストを取得
    auto slimes = Scene::Object::GetArray<Slime>();

    if(slimes.empty()) {
        // フィールド上にスライムが1体もいない場合、タイマーを進める
        slime_respawn_timer_ += GetDeltaTime();

        // 規定時間（5秒）経過したら再生成
        if(slime_respawn_timer_ >= SLIME_RESPAWN_TIME) {
            auto new_slime = Scene::Object::Create<Slime>();

            // 初期出現位置を設定（例: 空中から降ってくるように配置）
            if(new_slime) {
                new_slime->SetTranslate({0.0f, 10.0f, 0.0f});
            }

            // タイマーをリセット
            slime_respawn_timer_ = 0.0f;
        }
    }
    else {
        // スライムが存在している間はタイマーをリセットしておく
        slime_respawn_timer_ = 0.0f;
    }

    // デバッグ表示
    printfDx("\n DEAD ENEMY: %d", enemy_dead_count);
    if(slimes.empty()) {
        printfDx("\n SLIME RESPAWN IN: %.1f", SLIME_RESPAWN_TIME - slime_respawn_timer_);
    }

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
    //    }

    //    if(canCreateEnemy) {
    //        createEnemy();

    //        for(size_t i = 0; i < objs.size(); i++) {
    //            Scene::Object::Release(objs[i]);
    //        }
    //    }
    //}
}

//! @brief GUI表示
void PoittersPoint_Stage::GUI()
{
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
