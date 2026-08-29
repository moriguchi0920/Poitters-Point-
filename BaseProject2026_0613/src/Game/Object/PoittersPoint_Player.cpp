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
#include <System/Component/ComponentObjectController.h>

namespace PoittersPoint {

bool Player::Init()
{
    Super::Init();

    SetName("Player");

    AddComponent<ComponentModel>("data/Game/Models/Player/Player.mv1");
    if(auto model = GetComponent<ComponentModel>()) {
        model
            ->SetAnimation({
                {     "idle",        "data/Game/Models/Player/Anims/Idle.mv1", 1, 1.0f}, // Idle
                {     "walk",        "data/Game/Models/Player/Anims/Walk.mv1", 1, 1.0f}, // Walk
                {    "throw",  "data/Game/Models/Player/Anims/HandsThrow.mv1", 1, 1.0f}, // Throw
                {  "lift up", "data/Game/Models/Player/Anims/HandsLiftup.mv1", 1, 1.0f}, // liftup
                {"grab idle",    "data/Game/Models/Player/Anims/GrabIdle.mv1", 1, 1.0f}, // Throw
                {"grab walk",    "data/Game/Models/Player/Anims/GrabWalk.mv1", 1, 1.0f}, // Throw
        })
            ->SetScaleAxisXYZ({0.11f, 0.11f, 0.11f});
    }

    SetTranslate({0, 8, 0});

    // プレイヤー状態制御コンポーネントの追加
    AddComponent<ComponentPlayerState>();

    if(auto idle = AddComponent<ComponentStateIdleWalk>()) {
        idle->SetMoveSpeed(0.3f);
        idle->SetRotateSpeed(20.0f);
    }

    // カプセルコリジョンのコンポーネントを追加
    AddComponent<ComponentCollisionCapsule>();
    if(auto collision = GetComponent<ComponentCollisionCapsule>()) {
        collision->SetCollisionGroup(ComponentCollision::CollisionGroup::PLAYER);
        collision->UseGravity();
        collision->SetRadius(3.8f);
        collision->SetHeight(22.0f);
    }

    return true;
}

void Player::Update()
{
    Super::Update();

    // 視線・ターゲット検出処理
    OnEyeSight();
}

void Player::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    __super::OnHit(hit_info);
}

void Player::OnEyeSight()
{
    auto objects = Scene::Object::GetArray<Object>();

    float     shortest   = 1000.0f;
    ObjectPtr target_obj = nullptr;

    float3 playerPos = GetTranslate();

    for(auto& obj : objects) {
        if(!obj || obj == static_cast<ObjectPtr>(shared_from_this()))
            continue;

        auto grabbable = obj->GetComponent<ComponentGrabbable>();
        if(!grabbable)
            continue;

        float3 targetPos = obj->GetTranslate();
        float3 targetVec = targetPos - playerPos;

        float distance = sqrtf(targetVec.x * targetVec.x + targetVec.z * targetVec.z);

        if(distance <= 100.0f && distance < shortest) {
            if(grabbable->GetCanGrab()) {
                shortest   = distance;
                target_obj = obj;
            }
        }
    }

    if(target_obj) {
        if(auto player_state = GetComponent<ComponentPlayerState>()) {
            player_state->GrabbableHit(target_obj);
        }
    }
}

}    // namespace PoittersPoint
