//---------------------------------------------------------------------------
//! @file   PoittersPoint_Enemy.cpp
//! @brief  PoittersPoint_Enemy
//---------------------------------------------------------------------------
#include "PoittersPoint_Enemy.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "Game/Scene/PoittersPoint_GameOver.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentHitPoints.h>
#include <Game/Component/ComponentStateDead.h>

namespace PoittersPoint {

bool Enemy::Init()
{
    Super::Init();

    SetName("Enemy");

    AddComponent<ComponentModel>("data/Game/Models/Enemy/Enemy.mv1");
    if(auto model = GetComponent<ComponentModel>()) {
        model
            ->SetAnimation({
                {"walk",  "data/Game/Models/Enemy/Anims/Walk.mv1", 1, 1.0f}, // Walk
                { "run",   "data/Game/Models/Enemy/Anims/Run.mv1", 1, 1.0f}, // Run
                {"dead", "data/Game/Models/Enemy/Anims/Death.mv1", 1, 1.0f}, // dead
        })
            ->PlayAnimation("walk", true);

        model->SetScaleAxisXYZ({0.12f, 0.12f, 0.12f});
    }

    auto randomRange = [](int start, int end) { return start + rand() % ((end - start) + 1); };

    SetTranslate({(float)randomRange(-150, 150), 50.0f, (float)randomRange(-150, 150)});

    // 重複追加を防ぐ安全な設定
    auto collision = GetComponent<ComponentCollisionCapsule>();
    if(!collision) {
        collision = AddComponent<ComponentCollisionCapsule>();
    }

    if(collision) {
        collision->SetCollisionGroup(ComponentCollision::CollisionGroup::ENEMY);
        collision->SetRadius(4.0f);
        collision->SetHeight(23.0f);
        collision->UseGravity();
    }

    auto grabbable = GetComponent<ComponentGrabbable>();
    if(!grabbable) {
        grabbable = AddComponent<ComponentGrabbable>();
    }
    grabbable->SetLiftTime(0.5f);
    grabbable->SetDamage(1.0f);

    auto hp = GetComponent<ComponentHitPoints>();
    if(!hp) {
        hp = AddComponent<ComponentHitPoints>();
    }
    hp->SetMaxAndCurrentHP(2.0f);

    return true;
}

//============================================================================
// 更新処理
//============================================================================
void Enemy::Update()
{
    Super::Update();

    auto player = Scene::Object::Get<Object>("Player");

    if(player && is_down == false && is_dead == false) {
        float3 vec    = player->GetTranslate() - GetTranslate();
        auto   length = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        auto   model  = GetComponent<ComponentModel>();

        SetRotationToPositionWithLimit(player->GetTranslate(), 3.0f);

        if(is_running) {
            if(length < 35.0f) {
                is_running = false;
            }
        }
        else {
            if(45.0f <= length) {
                is_running = true;
            }
        }

        if(is_running) {
            model->PlayAnimationNoSame("run", true, 0.7f);
            AddTranslate({0, 0, -0.3f}, true);
        }
        else {
            model->PlayAnimationNoSame("walk", true, 0.5f);
            AddTranslate({0, 0, -0.1f}, true);
        }
    }

    if(!is_dead) {
        if(auto model = GetComponent<ComponentModel>()) {
            if(!model->IsPlaying()) {
                is_dead = true;
            }
        }
    }
}

//============================================================================
// 当たり判定のコールバック
//============================================================================
void Enemy::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    auto hitter = hit_info.hit_collision_->GetOwner();

    if(auto grabbable = hitter->GetComponent<ComponentGrabbable>()) {
        if(grabbable->IsThrower(SharedThis())) {
            Super::OnHit(hit_info);
            return;
        }

        if(!grabbable->IsMoving()) {
            Super::OnHit(hit_info);
            return;
        }

        if(grabbable->IsAlreadyHit(SharedThis())) {
            Super::OnHit(hit_info);
            return;
        }

        grabbable->AddHitTarget(SharedThis());
    }
    else {
        Super::OnHit(hit_info);
        return;
    }

    auto now_scene = Scene::GetCurrentScene();
    if(auto scene = dynamic_cast<PoittersPoint_Stage*>(now_scene)) {
        if(auto hp = GetComponent<ComponentHitPoints>()) {
            hp->TakeDamage(hitter->GetComponent<ComponentGrabbable>()->GetDamage());

            if(hp->GetHitPoints() <= 0.0f) {
                is_down = true;

                if(auto model = GetComponent<ComponentModel>()) {
                    AddComponent<ComponentStateDead>();
                }

                if(auto col = GetComponent<ComponentCollisionCapsule>()) {
                    RemoveComponent<ComponentCollisionCapsule>();
                }
            }
        }
    }

    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
