//---------------------------------------------------------------------------
//! @file   PoittersPoint_Character.cpp
//! @brief  PoittersPoint_Character
//---------------------------------------------------------------------------
#include "PoittersPoint_Character.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/StateMachine/ComponentStateMachine.h>

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
bool Character::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    return true;
}

void Character::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    auto target = hit_info.hit_collision_->GetOwnerPtr();
    if(auto grabbable = target->GetComponent<ComponentGrabbable>()) {
        auto state_machine = GetComponent<ComponentStateMachine>();
        state_machine->GrabbableHit(target);
    }

    __super::OnHit(hit_info);
}

void Character::SetCharacterStatus(const CharacterData& data)
{
    // 代入
    model_path_      = data.model_path_;
    anims_           = data.anims_;
    local_scale_     = data.local_scale_;
    move_speed_      = data.move_speed_;
    throw_power_     = data.throw_power_;
    lift_time_ratio_ = data.lift_time_ratio_;
    // モデルを入力されたパスやアニメーションで初期化
    AddComponent<ComponentModel>(model_path_)->SetAnimation(anims_)->SetScaleAxisXYZ(local_scale_);
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
}

float Character::GetMoveSpeed()
{
    return move_speed_;
}

}    // namespace PoittersPoint
