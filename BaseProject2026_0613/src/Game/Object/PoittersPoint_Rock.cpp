//---------------------------------------------------------------------------
//! @file   PoittersPoint_Rock.cpp
//! @brief  PoittersPoint_Rock
//---------------------------------------------------------------------------
#include "PoittersPoint_Rock.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentPhysics.h>
#include "Game/Component/ComponentGrabbable.h"

namespace PoittersPoint {
// namespace PoittersPoint

bool Rock::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetTranslate({10, 10, 10});

    SetName("Rock");
    AddComponent<ComponentModel>("data/Game/Models/Rock/rock.mv1");

    auto col = AddComponent<ComponentCollisionSphere>();

    col->SetRadius(5.0f);
    // 移動（重力）は下の ComponentPhysics に任せる。
    // ComponentCollision は「敵・プレイヤーへの当たり検知」だけを残す役割（重力の二重適用を防ぐため UseGravity しない）。
    col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                              (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
    col->SetMass(500.0f);

    // Jolt 剛体（ Tutorial_13 のドラム缶と同じ）。重力・慣性・摩擦・跳ね・転がりは Jolt が解く。
    auto physics = AddComponent<ComponentPhysics>();
    if(physics) {
        physics->CreateSphere({0.0f, 0.0f, 0.0f}, 5.0f);
        physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Actived, true);    // 重力有効
        physics->SetPhysicsReal(1.0f);                                             // 物理 100%
        physics->SetCollisionType(ComponentPhysics::CollisionType::ITEM);
        physics->SetFriction(0.5f);       //!< 摩擦（滑りの調整は GUI の「摩擦力」でも可）
        physics->SetRestitution(0.05f);   //!< 跳ね返り係数（石なのでほぼ跳ねない。大きすぎると静止接触でもマイクロ跳ね→上下カクツキの原因）

        // 速度減衰（指数的減衰。0で無効）。
        // Jolt の転がり摩擦は弱く放置だとほぼ永久に転がるため掛ける。
        // 直線減衰は GROUND 接地中のみ適用＝空中の飛距離は殺さず、着地後は素早く止まる。
        physics->SetLinearDamping(3.0f);     //!< 着地後の並進減衰（接地時のみ）
        physics->SetAngularDamping(2.0f);    //!< 転がりそのものへの抵抗（転がりすぎはここを上げる）
    }

    auto grabbable = AddComponent<ComponentGrabbable>();
    grabbable->SetBounceOffset(0.2f);
    grabbable->SetLiftTime(1.0f);
    grabbable->SetDamage(1.0f);    // ダメージ設定

    //AddComponent<ComponentCollisionModel>();
    //if(auto collision = GetComponent<ComponentCollisionModel>()) {
    //    collision->AttachToModel();    // コリジョンをモデルに合わせる
    //}

    SetScaleAxisXYZ({1.0f});

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
