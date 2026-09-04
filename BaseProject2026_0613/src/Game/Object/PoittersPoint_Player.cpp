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
#include "Game/Component/ComponentHitPoints.h"

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
    // 他者に掴まれて投げられるようにする
    AddComponent<ComponentGrabbable>()->SetLiftTime(0.5f);

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
    auto hitter = hit_info.hit_collision_->GetOwner();

    // ヒットした相手が掴めるもの(=投げられて移動中)か判定
    if(auto grabbable = hitter->GetComponent<ComponentGrabbable>()) {
        // 投げた本人自身の場合はダメージなし
        if(grabbable->IsThrower(SharedThis())) {
            __super::OnHit(hit_info);
            return;
        }

        // 地面で停止中はダメージなし
        if(!grabbable->IsMoving()) {
            __super::OnHit(hit_info);
            return;
        }

        // 既にこのターゲットをHit済みならスルー(連続ヒット防止)
        if(grabbable->IsAlreadyHit(SharedThis())) {
            __super::OnHit(hit_info);
            return;
        }

        // 初Hitなら記録してダメージ処理へ
        grabbable->AddHitTarget(SharedThis());
    }
    else {
        // 弾などgrabbable以外は今まで通り
        __super::OnHit(hit_info);
        return;
    }

    // 投げ物ごとのダメージ値を適用
    if(auto hp = GetComponent<ComponentHitPoints>()) {
        hp->TakeDamage(hitter->GetComponent<ComponentGrabbable>()->GetDamage());
    }
    __super::OnHit(hit_info);
}

void Player::OnEyeSight()
{
    auto   ObjArray = Scene::Object::GetArray<Object>();
    float3 vec;
    if(auto model = GetComponent<ComponentModel>()) {
        vec = -model->GetWorldVectorAxisZ();
        normalize(vec);
    }

    // 親から敵へのベクトル
    float3 targetVec = {0, 0, 0};
    // 一番近い敵への距離を保存する変数
    float shortest = 1000.0f;
    // 範囲for
    for(auto& obj : ObjArray) {
        if(obj == static_cast<ObjectPtr>(shared_from_this())) {
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
