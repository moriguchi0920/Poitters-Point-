#pragma once
#include <Game/Component/ComponentGrabbable.h>
#include "ComponentStateIdleWalk.h"
#include "Game/Scene/PoittersPoint_Stage.h"

void ComponentGrabbable::Init()
{
    __super::Init();

    // GetOnwer：オーナー(自分がAddComponentされたObject)を取得します
    // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
    auto owner     = GetOwner();
    is_grabbed_    = false;
    can_grab_this_ = true;
}

void ComponentGrabbable::Update()
{
    __super::Update();
    auto owner = GetOwner();
    if(!is_grabbed_) {
        owner->AddTranslate(translation_);
        translation_ = lerp(translation_, float3(0.0f, 0.0f, 0.0f), 0.03);
    }
    if(IsMoving()) {
        can_grab_this_ = false;
    }
    else if(!is_grabbed_) {
        can_grab_this_ = true;
    }
}

void ComponentGrabbable::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("Grabbable")) {
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

void ComponentGrabbable::SetLiftTime(float time)
{
    lift_time_ = time;
}

const float& ComponentGrabbable::GetLiftTime() const
{
    return lift_time_;
}

void ComponentGrabbable::SetTranslation(float3 new_translation)
{
    translation_ = new_translation;
}

void ComponentGrabbable::AddTranslation(float3 add_translation)
{
    translation_ += add_translation;
}

const float3& ComponentGrabbable::GetTranslation()
{
    return translation_;
}

void ComponentGrabbable::SetIsGrabbed(bool grabbed)
{
    is_grabbed_ = grabbed;
}

bool ComponentGrabbable::GetIsGrabbed()
{
    return is_grabbed_;
}

bool ComponentGrabbable::GetCanGrab()
{
    return can_grab_this_;
}

void ComponentGrabbable::SetCanGrab(const bool can_grab)
{
    can_grab_this_ = can_grab;
}

void ComponentGrabbable::SetBounceOffset(float offset)
{
    bounce_offset_ = offset;
}

void ComponentGrabbable::Bounce()
{
    AddTranslation({0.0f, bounce_offset_, 0.0f});
    bounce_offset_ -= 0.03f;
}

bool ComponentGrabbable::IsGrounded()
{
    return bounce_offset_ < 0.0f;
}

bool ComponentGrabbable::IsMoving()
{
    return ((translation_.x < -0.01f || 0.01f < translation_.x) || (translation_.z < -0.01f || 0.01f < translation_.z));
}

CEREAL_REGISTER_TYPE(ComponentGrabbable)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentGrabbable)
