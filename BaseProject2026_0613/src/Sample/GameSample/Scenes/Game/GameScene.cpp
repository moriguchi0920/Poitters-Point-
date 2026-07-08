#include "GameScene.h"
#include "Objects/Mouse.h"
#include "Objects/Enemy.h"
#include "Objects/Box.h"
#include "Objects/Camera.h"

#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>

namespace Sample::GameSample {

// STAGE01を使用するときはこちらを有効にする
//#define USE_STAGE_FPS
//#define USE_MOUSE_CAMERA

bool GameScene::Init()
{
#ifdef USE_STAGE_FPS
    {
        auto obj = Scene::Object::Create<Object>()->SetName("Ground");
        obj->AddComponent<ComponentModel>("data/Sample/FPS_Industrial/Map.mv1")->SetScaleAxisXYZ({5, 5, 5});
        obj->AddComponent<ComponentCollisionModel>()->AttachToModel();
    }
#else
    {
        auto obj = Scene::Object::Create<Object>()->SetName("Ground");
        obj->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
        if(auto cmp_mdl = obj->AddComponent<ComponentCollisionModel>())
            cmp_mdl->AttachToModel(true);
    }
#endif

    //----------------------------------------------------------------------------------
    // Mouse
    //----------------------------------------------------------------------------------
    auto m = Mouse::Create({0, 100, 0});

    m->SetSpeed(0.5f);

    //----------------------------------------------------------------------------------
    // 箱
    //----------------------------------------------------------------------------------
    Box::Create({0, 10, 0});

    //----------------------------------------------------------------------------------
    // Enemy
    //----------------------------------------------------------------------------------
    auto enemy = Enemy::Create({GetRand(1000) - 500, 100, -GetRand(500)});
    enemy->SetSpeed(0.1f);

    // 敵のカメラ
    Camera::Create(enemy)->SetName("EnemyCamera");

    for(int i = 0; i < 10; i++) {
        enemy = Enemy::Create({GetRand(1000) - 500, 100, -GetRand(500)});
        enemy->SetSpeed(0.1f);
    }

    // プレイヤーカメラを最後に作成して優先を上げておく
    Camera::Create(m)->SetName("PlayerCamera");

    return true;
}

void GameScene::Update()
{
    if(IsKeyOn(KEY_INPUT_1))
        Scene::SetCurrentCamera("PlayerCamera");

    if(IsKeyOn(KEY_INPUT_2))
        Scene::SetCurrentCamera("EnemyCamera");

#if 1
    // 10秒たったら敵を速くする
    if(Scene::GetTime() > 10.0f) {
        auto enemies = Scene::GetObjectsPtr<Enemy>();
        for(auto& enemy : enemies) {
            enemy->SetSpeed(0.4f);
        }
    }
#endif
}

void GameScene::Draw()
{
    // とりあえずTitleという文字を表示しておく
    DrawFormatString(100, 50, GetColor(255, 255, 255), "Title");
}

void GameScene::Exit()
{
    // タイトル終了時に行いたいことは今はない
}

void GameScene::GUI()
{
}

}    // namespace Sample::GameSample
