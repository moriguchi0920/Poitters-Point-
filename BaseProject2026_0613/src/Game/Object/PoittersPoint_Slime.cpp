#include "PoittersPoint_Slime.h"

#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>

#include <System/Component/ComponentModel.h>

#include <System/Component/ComponentCollisionSphere.h>

#include "Game/Component/ComponentGrabbable.h"

namespace PoittersPoint {

bool Slime::Init()

{
    if(!Super::Init())

        return false;

    SetName("Slime");

    // 1. スケールは一旦 1.0f で標準化（極小化によるコリジョン不具合を防ぐ）

    SetScaleAxisXYZ({0.2f, 0.2f, 0.2f});

    SetTranslate({0.0f, 6.0f, 12.0f});

    // 2. 3Dモデルの設定

    auto model = AddComponent<ComponentModel>("data/Game/Models/Slime/Slime.mv1");

    // 3. 当たり判定（球体）の設定
    ComponentCollisionSphere* col = AddComponent<ComponentCollisionSphere>().get();
    if(col) {
        col->SetRadius(5.0f);
        col->UseGravity(true);
        col->SetGravity(-0.2f);
        col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
        // ★ GROUNDだけでなくETCなども含めて衝突を受け取れるように調整
        col->SetHitCollisionGroup(static_cast<u32>(ComponentCollision::CollisionGroup::GROUND) | static_cast<u32>(ComponentCollision::CollisionGroup::PLAYER) |
                                  static_cast<u32>(ComponentCollision::CollisionGroup::ETC) | static_cast<u32>(ComponentCollision::CollisionGroup::WEAPON));
        col->SetMass(5.0f);
    }

    // 4. 持ち上げ機能（Grabbable）
    auto grabbable = AddComponent<ComponentGrabbable>();
    if(grabbable) {
        grabbable->SetBounceOffset(0.2f);
        grabbable->SetLiftTime(1.0f);
        grabbable->SetCanGrab(true);    // ★ 初期化時 true
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
    if(hit_info.hit_collision_ && hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
        if(auto grabbable = GetComponent<ComponentGrabbable>()) {
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
