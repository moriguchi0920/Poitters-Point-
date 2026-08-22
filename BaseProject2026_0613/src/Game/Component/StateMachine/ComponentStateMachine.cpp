#pragma once
#include <Game/Component/StateMachine/ComponentStateMachine.h>
#include "Game/Scene/PoittersPoint_Stage.h"
#include <Game/Component/ComponentGrabbable.h>

void ComponentStateMachine::Init()
{
    __super::Init();
}

void ComponentStateMachine::Update()
{
    __super::Update();
}

void ComponentStateMachine::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State Machine")) {
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

            //if(ImGui::TreeNode("State IdleWalk")) とセット

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

const std::string ComponentStateMachine::GetStateName() const
{
    if(auto state = GetOwner()->GetComponent<ComponentState>()) {
        return state->GetName().data();
    }
    return "";
}

void ComponentStateMachine::GrabbableHit(ObjectPtr target)
{
    if(auto owner = GetOwner()) {
        if(auto grabbable = target->GetComponent<ComponentGrabbable>()) {
            if(grabbable->GetCanGrab() && can_grab_) {
                grabbing_object_ptr_ = target;
            }
        }
    }
}

bool ComponentStateMachine::GetCanGrab()
{
    return can_grab_;
}

bool ComponentStateMachine::GetGrabbing()
{
    return !grabbing_object_ptr_.expired() && !can_grab_;
}

CEREAL_REGISTER_TYPE(ComponentStateMachine)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateMachine)
