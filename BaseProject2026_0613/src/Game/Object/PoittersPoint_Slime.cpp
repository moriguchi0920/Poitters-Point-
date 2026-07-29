//---------------------------------------------------------------------------
//! @file   PoittersPoint_Slime.cpp
//! @brief  PoittersPoint_Slime
//---------------------------------------------------------------------------
#include "PoittersPoint_Slime.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"

namespace PoittersPoint {
// namespace PoittersPoint

bool Slime::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetTranslate({10, 10, 10});

    SetName("Slime");
    AddComponent<ComponentModel>("data/Game/Models/model.obj");

    auto col = AddComponent<ComponentCollisionSphere>();

    col->SetRadius(5.0f)->UseGravity(true);
    col->SetGravity(-0.2f);
    col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                              (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
    col->SetMass(500.0f);

    auto grabbable = AddComponent<ComponentGrabbable>();
    grabbable->SetBounceOffset(0.2f);
    grabbable->SetLiftTime(1.0f);

    //AddComponent<ComponentCollisionModel>();
    //if(auto collision = GetComponent<ComponentCollisionModel>()) {
    //    collision->AttachToModel();    // コリジョンをモデルに合わせる
    //}

    SetScaleAxisXYZ({1.0f});

    return true;
}

void Slime::Update()
{
    __super::Update();
}

void Slime::GUI()
{
    __super::GUI();
}

void Slime::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    /*
    // 当たった相手の名前がEnemyだったら消去する
    auto name = hit_info.hit_collision_->GetOwner()->GetNameDefault();
    if(name == "Enemy") {
        hit_info.collision_->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, true);
        // 自分を削除する
        Scene::Object::Release(SharedThis());
    }
    */
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

    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
