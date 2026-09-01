#pragma once
#include <Game/Component/State/ComponentStateTargetWalk.h>

void ComponentStateTargetWalk::Init()
{
    __super::Init();
    SetName<Component>("State Target Walk");
    ResetTargetPtr();
}

void ComponentStateTargetWalk::Update()
{
    __super::Update();

    // オーナー(自分がAddComponentされたObject)を取得します
    // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
    auto owner = GetOwner();

    if(target_ptr_) {
        target_pos_ = target_ptr_->GetTranslate();
    }

    // 移動方向
    float3 dir{0, 0, 0};

    dir   = target_pos_ - owner->GetTranslate();
    dir.y = 0.0f;

    // 移動キーが押されているか?
    if((float)length(dir) > 0.1f) {
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

ComponentStateTargetWalkPtr ComponentStateTargetWalk::SetTargetPos(const float3& position)
{
    target_pos_ = position;
    return std::dynamic_pointer_cast<ComponentStateTargetWalk>(shared_from_this());
}

void ComponentStateTargetWalk::SetTargetPtr(const ObjectPtr ptr)
{
    target_ptr_ = ptr;
}

void ComponentStateTargetWalk::ResetTargetPtr()
{
    target_ptr_ = nullptr;
}

const float3& ComponentStateTargetWalk::GetTargetPos()
{
    return target_pos_;
}

bool ComponentStateTargetWalk::GetArrival()
{
    auto owner     = GetOwner();
    auto translate = owner->GetTranslate();
    auto vec       = target_pos_ - translate;
    return fabsf(vec.x) < 0.1 && fabsf(vec.y) < 0.1 && fabsf(vec.z) < 0.1;
}

void ComponentStateTargetWalk::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State TargetWalk")) {
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
            ImGui::DragFloat3(u8"目的地", target_pos_.RawArray(), 1.0f);

            // 移動の基本情報
            ImGui::DragFloat(u8"オブジェクト オフセット回転", &front_rot_, 1.0f);

            //if(ImGui::TreeNode("State IdleWalk")) とセット
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentStateTargetWalk)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateTargetWalk)
