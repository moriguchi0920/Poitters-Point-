//---------------------------------------------------------------------------
//! @file   PoittersPoint_Slime.cpp
//! @brief  PoittersPoint_Slime
//---------------------------------------------------------------------------
#include "PoittersPoint_Slime.h"
#include "PoittersPoint_Slime_Explosion.h"    // SlimeExplosionの型定義を読み込む
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include "Game/Component/ComponentGrabbable.h"
#include "Game/Component/ComponentHitPoints.h"

namespace PoittersPoint {

bool Slime::Init()
{
    if(!Super::Init())
        return false;

    SetName("Slime");

    // スケール・位置設定
    SetScaleAxisXYZ({0.2f, 0.2f, 0.2f});
    SetTranslate({0.0f, 6.0f, 12.0f});

    // 3Dモデルの設定
    auto model = AddComponent<ComponentModel>("data/Game/Models/Slime/Slime.mv1");

    // 当たり判定（球体）の設定
    ComponentCollisionSphere* col = AddComponent<ComponentCollisionSphere>().get();
    if(col) {
        col->SetRadius(5.0f);
        col->UseGravity(true);
        col->SetGravity(-0.2f);
        col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
        col->SetHitCollisionGroup(static_cast<u32>(ComponentCollision::CollisionGroup::GROUND) | static_cast<u32>(ComponentCollision::CollisionGroup::PLAYER) |
                                  static_cast<u32>(ComponentCollision::CollisionGroup::ETC) | static_cast<u32>(ComponentCollision::CollisionGroup::WEAPON));
        col->SetMass(5.0f);
    }

    // 持ち上げ機能（Grabbable）
    auto grabbable = AddComponent<ComponentGrabbable>();
    if(grabbable) {
        grabbable->SetBounceOffset(0.2f);
        grabbable->SetLiftTime(1.0f);
        grabbable->SetCanGrab(true);
        grabbable->SetDamage(2.0f);
    }

    return true;
}

void Slime::Update()
{
    Super::Update();
}

void Slime::GUI()
{
    Super::GUI();
}

void Slime::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    if(auto grabbable = GetComponent<ComponentGrabbable>()) {
        // 投げられて移動中（飛行中）の場合
        if(grabbable->IsMoving()) {
            auto hitter = hit_info.hit_collision_->GetOwner();

            // 投げた本人との衝突判定はスキップ
            if(hitter && grabbable->IsThrower(hitter->SharedThis())) {
                Super::OnHit(hit_info);
                return;
            }

            // 地面または他のオブジェクトに当たったら爆発
            Explode();
            return;
        }

        // 通常の着地処理
        if(hit_info.hit_collision_ && hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
            grabbable->SetCanGrab(true);
            if(grabbable->IsGrounded()) {
                grabbable->SetBounceOffset(0.2f);
            }
        }
    }

    Super::OnHit(hit_info);
}

//===========================================================================
//! @brief 爆発処理
//===========================================================================
void Slime::Explode()
{
    float3 myPos           = GetTranslate();
    float  explosionRadius = 45.0f;
    float  explosionDamage = 2.0f;

    // 範囲ダメージ処理
    auto objArray = Scene::Object::GetArray<Object>();
    for(auto& obj : objArray) {
        if(!obj || obj == static_cast<ObjectPtr>(shared_from_this())) {
            continue;
        }

        float3 targetPos = obj->GetTranslate();
        float3 vec       = targetPos - myPos;
        float  distance  = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);

        if(distance <= explosionRadius) {
            if(auto hp = obj->GetComponent<ComponentHitPoints>()) {
                hp->TakeDamage(explosionDamage);
            }
        }
    }

    // ★爆発エフェクトの生成（Scene::Object::Create を使用）
    auto explosion = Scene::Object::Create<SlimeExplosion>();
    if(explosion) {
        explosion->SetTranslate(myPos);
    }

    // スライム自身の削除
    Scene::ReleaseObject(SharedThis());
}

}    // namespace PoittersPoint
