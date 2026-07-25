#pragma once
#include <Game/Component/StateMachine/ComponentCPUState.h>
#include <Game/Component/State/ComponentStateTargetWalk.h>

void ComponentCPUState::Init()
{
    __super::Init();
}

void ComponentCPUState::Update()
{
    __super::Update();
}

void ComponentCPUState::LateUpdate()
{
    can_grab_ = false;
}

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

void ComponentCPUState::GrabbableHit(bool is_hit_grabbable, ObjectPtr target)
{
    auto owner = GetOwner();
    if(owner->GetComponent<ComponentStateTargetWalk>()) {
        can_grab_            = is_hit_grabbable;
        grabbing_object_ptr_ = target;
    }
}

CEREAL_REGISTER_TYPE(ComponentCPUState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCPUState)
