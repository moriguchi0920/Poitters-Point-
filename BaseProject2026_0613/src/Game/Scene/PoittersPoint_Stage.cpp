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

    // ★ タイマーの初期化（制限時間をセット）
    PoittersPoint_Timer::SetMaxTime(120.0f);    // 例: 120秒（2分）
    PoittersPoint_Timer::SetTime(120.0f);
    PoittersPoint_Timer::SetPause(false);

    return true;
}

//! @brief 更新
void PoittersPoint_Stage::Update()
{
    counter2++;

    // ★ タイマーの更新処理
    PoittersPoint_Timer::Update();

    // 制限時間終了判定（タイムアップ時）
    if(PoittersPoint_Timer::IsTimeUp()) {
        // ★ シーン遷移処理
        // ご利用のフレームワークの仕様に合わせて選択してください
        // パターンA: テンプレートによる切り替え
        Scene::Change(Scene::GetScene<PoittersPoint_GameOver>());

        // ※もし上記で「型名は使用できません」が消えない場合、オブジェクトとして作成している可能性があります
        // パターンB: Scene::CreateScene / Scene::Change(Scene::GetScene<...>) の場合
        // Scene::Change(Scene::GetScene<PoittersPoint_GameOver>());

        return;
    }

    // テスト用
    if(auto obj = Scene::Object::Get<Object>("OBJ")) {
        obj->AddTranslate({0.001f, 0.0f, 0.0f});
    }

    printfDx("\n TIME: %.1f", PoittersPoint_Timer::GetTimer());
    printfDx("\n DEAD ENEMY: %d", enemy_dead_count);

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

    // GUIでもタイマーをデバッグ表示可能に
    float time = PoittersPoint_Timer::GetTimer();
    if(ImGui::InputFloat("Remaining Time", &time)) {
        PoittersPoint_Timer::SetTime(time);
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
