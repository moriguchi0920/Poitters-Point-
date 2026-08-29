#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentCPUState.cpp
//! @brief  CPU状態制御コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentCPUState.h>
#include "ComponentStateIdleWalk.h"
#include "ComponentStateGrab.h"
#include "ComponentStateThrow.h"
#include "ComponentGrabbable.h"

//===========================================================================
// 初期化処理
//===========================================================================
void ComponentCPUState::Init()
{
    __super::Init();
}

//===========================================================================
// 更新処理
//===========================================================================
void ComponentCPUState::Update()
{
    __super::Update();

    auto owner = GetOwner();

    // この処理にはComponentStateGrabが必要なので、掴み状態の時のみ処理する(今はここに入らない)
    if(auto component_grab = owner->GetComponent<ComponentStateGrab>()) {
        // 掴みアニメーションが終了したら、掴み状態を完了させる
        if(component_grab->GetFinished()) {
            if(!grabbing_object_ptr_.expired()) {
                auto object = grabbing_object_ptr_.lock();

                // 掴んでいる間は当たり判定を無効化
                if(auto collider = object->GetComponent<ComponentCollision>()) {
                    collider->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, true);
                }

                // 掴んでいる間は掴み可能状態を無効化
                auto grabbable = object->GetComponent<ComponentGrabbable>();
                grabbable->SetIsGrabbed(true);

                // Hit記録をクリア(投げ直したら誰にでもHitできるように)
                grabbable->ClearHitTargets();

                // 手にモデルをアタッチ
                grabbing_object_ptr_.lock()->AddComponent<ComponentAttachModel>()->SetAttachObject(owner->GetName(), "mixamorig:RightHand");
            }

            // 掴み状態を完了させる
            can_throw_ = true;
            ChangeState<ComponentStateIdleWalk>()->SetIsHolding(true);
        }
    }
}

//===========================================================================
// 更新後処理
//===========================================================================
void ComponentCPUState::LateUpdate()
{
    // 掴み可能フラグをリセット
    can_grab_ = false;
}

//===========================================================================
// GUI処理
//===========================================================================
void ComponentCPUState::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("CPU State")) {
            //-------------------------------------------------------
            // 共通部分(共通化したい)

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分(SampleObjectController)を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//===========================================================================
// 掴み可能物を検知すると呼ばれる判定
//===========================================================================
void ComponentCPUState::GrabbableHit(bool is_hit_grabbable, ObjectPtr target)
{
    auto owner = GetOwner();
    if(owner->GetComponent<ComponentStateIdleWalk>()) {
        can_grab_            = is_hit_grabbable;
        grabbing_object_ptr_ = target;
    }
}

CEREAL_REGISTER_TYPE(ComponentCPUState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCPUState)
