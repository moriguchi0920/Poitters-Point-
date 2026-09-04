#pragma once
#include <Game/Component/State/ComponentStateSetRangeWalk.h>

void ComponentStateSetRangeWalk::Init()
{
    __super::Init();
    SetName<Component>("State SetRangeWalk");
}

void ComponentStateSetRangeWalk::Update()
{
    auto owner = GetOwner();
    __super::Update();
    // 移動方向
    float3 dir = walk_direction_;


    if(walk_distance_ > 0.1f) {

        dir = normalize(dir);

        // キャラのローカル方向で移動をさせる
        owner->AddTranslate(dir * move_speed_, true);

        float3 move     = dir * move_speed_;
        walk_distance_ -= sqrtf(move.x * move.x + move.y * move.y + move.z + move.z);

        // モデルを移動の方向に向けます
        if(auto mdl = owner->GetComponent<ComponentModel>()) {
            auto rot = quaternion::rotation_axis({0, 1, 0}, front_rot_ * DegToRad);    //< Y軸1度回転

            mdl->SetRotationToVectorWithLimit(mul(dir, rot), rot_speed_);
            if(is_holding_) {
                mdl->PlayAnimationNoSame("grab walk", true);
            }
            else {
                mdl->PlayAnimationNoSame("walk", true);
            }
        }
    }
    else {
        stopping_time_ += GetDeltaTime();
        if(auto mdl = owner->GetComponent<ComponentModel>()) {
            if(is_holding_) {
                mdl->PlayAnimationNoSame("grab idle", true);
            }
            else {
                mdl->PlayAnimationNoSame("idle", true);
            }
        }
    }
}


void ComponentStateSetRangeWalk::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State SetRangeWalk")) {
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

void ComponentStateSetRangeWalk::SetWalkDirection(const float3& direction)
{
    walk_direction_ = direction;
}

void ComponentStateSetRangeWalk::SetWalkDistance(const float& distance)
{
    walk_distance_ = distance;
}

bool ComponentStateSetRangeWalk::GetArrival()
{
    return walk_distance_ < 0.1f;
}

bool ComponentStateSetRangeWalk::GetStopped()
{
    return stop_judge_time_ < stopping_time_;
}


CEREAL_REGISTER_TYPE(ComponentStateWalkBase)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateWalkBase)
