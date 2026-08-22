#pragma once
#include <Game/Component/State/ComponentStateGrab.h>
#include "ComponentStateControllerWalk.h"
#include "Game/Scene/PoittersPoint_Stage.h"

void ComponentStateGrab::Init()
{
    __super::Init();

    SetName<Component>("State Grab");

    auto owner = GetOwner();
    if(auto model = owner->GetComponent<ComponentModel>()) {
        model->PlayAnimationNoSame("lift up");
    }
}

void ComponentStateGrab::Update()
{
    lift_time_ -= GetDeltaTime();
    if(lift_time_ < 0.0f) {
        finished_  = true;
        auto owner = GetOwner();
        if(auto model = owner->GetComponent<ComponentModel>()) {
            model->PlayAnimationNoSame("grab idle");
        }
    }

    __super::Update();
}

void ComponentStateGrab::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State Grab")) {
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

void ComponentStateGrab::SetLiftTime(float time)
{
    lift_time_ = time;
}

bool ComponentStateGrab::GetIsFinished()
{
    return finished_;
}

CEREAL_REGISTER_TYPE(ComponentStateGrab)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateGrab)
