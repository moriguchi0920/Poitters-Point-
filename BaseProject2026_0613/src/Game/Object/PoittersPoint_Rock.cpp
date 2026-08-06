//---------------------------------------------------------------------------
//! @file   PoittersPoint_Rock.cpp
//! @brief  PoittersPoint_Rock
//---------------------------------------------------------------------------
#include "PoittersPoint_Rock.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"

namespace PoittersPoint {

bool Rock::Init()
{
    Super::Init();

    SetName("Rock");

    // モデル読み込み
    auto modelComp = AddComponent<ComponentModel>();
    if(modelComp) {
        modelComp->UseShader(false);
        modelComp->Load("data/Game/Models/Rock/rock.mv1");
    }

    // 当たり判定の追加
    auto col = AddComponent<ComponentCollisionSphere>();

    col->SetRadius(5.0f)->UseGravity(true);

    col->SetGravity(-0.25f);

    col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                              (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);

    col->SetMass(10.0f);

    auto grabbable = AddComponent<ComponentGrabbable>();
    grabbable->SetBounceOffset(0.2f);
    grabbable->SetLiftTime(1.0f);

    SetScaleAxisXYZ({1.0f, 1.0f, 1.0f});

    return true;
}

void Rock::Update()
{
    __super::Update();
}

void Rock::GUI()
{
    __super::GUI();
}

void Rock::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    if(hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
        if(auto grabbable = GetComponent<ComponentGrabbable>()) {
            float3 translation = grabbable->GetTranslation();
            grabbable->SetCanGrab(true);

            if(grabbable->IsGrounded()) {
                grabbable->SetBounceOffset(0.2f);
            }
            else if(grabbable->IsMoving()) {
                grabbable->Bounce();
            }
        }
    }

    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
