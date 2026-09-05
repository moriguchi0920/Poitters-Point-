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

    // Enemyという名前がついたObjectをVectorで取得
    auto enemies   = Scene::Base::GetObjectsPtr<Object>("Enemy");
    auto enemy_num = enemies.size();

    // 死亡カウント更新
    auto released_enemy_num = MAX_ENEMIES - static_cast<int>(enemy_num);
    enemy_dead_count        = released_enemy_num;

    // 敵が全滅した場合の再生成処理
    if(MAX_ENEMIES <= released_enemy_num) {
        bool canCreateEnemy = true;
        auto objs           = Scene::Object::GetArray<Enemy>();
        for(size_t i = 0; i < objs.size(); i++) {
            if(objs[i]->GetName() == "Enemy" || objs[i]->is_dead == false) {
                canCreateEnemy = false;
            }
        }

        if(canCreateEnemy) {
            createEnemy();

            for(size_t i = 0; i < objs.size(); i++) {
                Scene::Object::Release(objs[i]);
            }
        }
    }
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
