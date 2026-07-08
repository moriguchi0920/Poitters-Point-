//---------------------------------------------------------------------------
//! @file   PoittersPoint_Player.cpp
//! @brief  PoittersPoint_Player
//---------------------------------------------------------------------------
#include "PoittersPoint_Player.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "PoittersPoint_Bullet.h"
#include "Game/Component/ComponentCameraController.h"
#include "Game/Component/ComponentStateIdleWalk.h"
#include "Game/Component/ComponentStateThrow.h"
#include "Game/Component/ComponentPlayerState.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
bool Player::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetName("Player");
    //auto player = Scene::Object::Create<Object>("Player");

    AddComponent<ComponentModel>("data/Game/Models/Player/Player.mv1");
    if(auto model = GetComponent<ComponentModel>()) {
        model
            ->SetAnimation({
                //                                  モデルビユーで何番目にいたか
                //                                                 ↓
                { "idle",  "data/Game/Models/Player/Anims/Idle.mv1", 1, 1.0f}, // Idle
                { "walk",  "data/Game/Models/Player/Anims/Walk.mv1", 1, 1.0f}, // Walk
                {"throw", "data/Game/Models/Player/Anims/Throw.mv1", 1, 1.0f}, // Throw
        })
            //->SetScaleAxisXYZ({0.12f, 0.12f, 0.12f});
            ->SetScaleAxisXYZ({0.11f, 0.11f, 0.11f});
    }

    SetTranslate({0, 5, 0});

    AddComponent<ComponentPlayerState>();

    // カメラの制御を行うコンポーネントを追加
    AddComponent<ComponentCameraController>();

    // カプセルコリジョンのコンポーネントを追加
    AddComponent<ComponentCollisionCapsule>();
    if(auto collision = GetComponent<ComponentCollisionCapsule>()) {
        // 所属するグループを「PLAYER」とします
        collision->SetCollisionGroup(ComponentCollision::CollisionGroup::PLAYER);

        // Collisionの重力を有効にします
        collision->UseGravity();
        collision->SetRadius(3.8f);     // コリジョンの半径を3.0 にする
        collision->SetHeight(22.0f);    // コリジョンの高さを13.0 にする
    }

    return true;
}

void Player::Update()
{
}

}    // namespace PoittersPoint
