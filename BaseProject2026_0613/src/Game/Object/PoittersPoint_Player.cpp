//---------------------------------------------------------------------------
//! @file   PoittersPoint_Player.cpp
//! @brief  PoittersPoint_Player
//---------------------------------------------------------------------------
#include "PoittersPoint_Player.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "Game/Component/ComponentCameraController.h"
#include "Game/Component/ComponentStateIdleWalk.h"
#include "Game/Component/ComponentStateThrow.h"
#include "Game/Component/ComponentPlayerState.h"
#include "Game/Component/ComponentStateMachine.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"

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
                {     "idle",        "data/Game/Models/Player/Anims/Idle.mv1", 1, 1.0f}, // Idle
                {     "walk",        "data/Game/Models/Player/Anims/Walk.mv1", 1, 1.0f}, // Walk
                {    "throw",  "data/Game/Models/Player/Anims/HandsThrow.mv1", 1, 1.0f}, // Throw
                {  "lift up", "data/Game/Models/Player/Anims/HandsLiftup.mv1", 1, 1.0f}, // liftup
                {"grab idle",    "data/Game/Models/Player/Anims/GrabIdle.mv1", 1, 1.0f}, // Throw
                {"grab walk",    "data/Game/Models/Player/Anims/GrabWalk.mv1", 1, 1.0f}, // Throw
        })
            //->SetScaleAxisXYZ({0.12f, 0.12f, 0.12f});
            ->SetScaleAxisXYZ({0.11f, 0.11f, 0.11f});
    }

    SetTranslate({0, 8, 0});

    AddComponent<ComponentPlayerState>();

    // カメラの制御を行うコンポーネントを追加
    //AddComponent<ComponentCameraController>();

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

void Player::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    auto target = hit_info.hit_collision_->GetOwnerPtr();
    if(auto grabbable = target->GetComponent<ComponentGrabbable>()) {
        auto player_state = GetComponent<ComponentPlayerState>();
        if (player_state->can_grab_ && grabbable->GetCanGrab())
        {
            player_state->GrabbableHit(target);
        }

    }

    __super::OnHit(hit_info);
}

}    // namespace PoittersPoint
