#pragma once
#include <Game/Component/State/ComponentStateWalkBase.h>

void ComponentStateWalkBase::Init()
{
    __super::Init();
    SetName<Component>("State IdleWalk");
}

void ComponentStateWalkBase::Update()
{
    __super::Update();
}

ComponentStateWalkBasePtr ComponentStateWalkBase::SetMoveSpeed(const float speed)
{
    move_speed_ = speed;
    return std::dynamic_pointer_cast<ComponentStateWalkBase>(shared_from_this());
}
ComponentStateWalkBasePtr ComponentStateWalkBase::SetRotateSpeed(const float speed)
{
    rot_speed_ = speed;
    return std::dynamic_pointer_cast<ComponentStateWalkBase>(shared_from_this());
}

const float ComponentStateWalkBase::GetMoveSpeed() const
{
    return move_speed_;
}

const float ComponentStateWalkBase::GetRotateSpeed() const
{
    return rot_speed_;
}

void ComponentStateWalkBase::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State WalkBase")) {
            //-------------------------------------------------------
            // 共通部分

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分(SampleObjectController)を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            // 移動の基本情報
            ImGui::DragFloat(u8"移動速度", &move_speed_, 0.1f);
            ImGui::DragFloat(u8"移動回転角度", &rot_speed_, 1.0f);

            // 移動の基本情報
            ImGui::DragFloat(u8"オブジェクト オフセット回転", &front_rot_, 1.0f);

            //if(ImGui::TreeNode("State IdleWalk")) とセット
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentStateWalkBase::SetIsHolding(bool hold)
{
    is_holding_ = hold;
}

CEREAL_REGISTER_TYPE(ComponentStateWalkBase)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateWalkBase)
