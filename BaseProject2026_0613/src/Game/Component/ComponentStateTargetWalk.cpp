#pragma once
#include <Game/Component/ComponentStateTargetWalk.h>

void ComponentStateTargetWalk::Init()
{
    __super::Init();
    SetName<Component>("State Target Walk");
}

void ComponentStateTargetWalk::Update()
{
    __super::Update();

    // オーナー(自分がAddComponentされたObject)を取得します
    // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
    auto owner = GetOwner();

    // 移動方向
    float3 dir{0, 0, 0};

    dir = target_pos_ - owner->GetTranslate();

    // 移動キーが押されているか?
    if((float)length(dir) > 0.0f) {
        // 斜めが押されていることを考慮し、
        // その方向の移動スピードを1とし、スピードを掛け合わせる
        dir = normalize(dir);

        // キャラのローカル方向で移動をさせる
        owner->AddTranslate(dir * move_speed_, true);

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

ComponentStateTargetWalkPtr ComponentStateTargetWalk::SetMoveSpeed(const float speed)
{
    move_speed_ = speed;
    return std::dynamic_pointer_cast<ComponentStateTargetWalk>(shared_from_this());
}
ComponentStateTargetWalkPtr ComponentStateTargetWalk::SetRotateSpeed(const float speed)
{
    rot_speed_ = speed;
    return std::dynamic_pointer_cast<ComponentStateTargetWalk>(shared_from_this());
}

const float ComponentStateTargetWalk::GetMoveSpeed() const
{
    return move_speed_;
}

const float ComponentStateTargetWalk::GetRotateSpeed() const
{
    return rot_speed_;
}

ComponentStateTargetWalkPtr ComponentStateTargetWalk::SetKeys(int up, int down, int left, int right)
{
    // 移動キーの設定
    key_up_    = up;
    key_down_  = down;
    key_left_  = left;
    key_right_ = right;
    return std::dynamic_pointer_cast<ComponentStateTargetWalk>(shared_from_this());
}

void ComponentStateTargetWalk::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State IdleWalk")) {
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

void ComponentStateTargetWalk::SetIsHolding(bool hold)
{
    is_holding_ = hold;
}

CEREAL_REGISTER_TYPE(ComponentStateTargetWalk)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateTargetWalk)
