#pragma once
#include <Game/Component/ComponentStatePlayerJump.h>
#include <Game/Component/ComponentStateIdleWalk.h>

void ComponentStatePlayerJump::Init()
{
    __super::Init();
    SetName<Component>("State PlayerJump");

    auto owner = GetOwner();
    if(!owner)
        return;

    // 踏切時点の地面の高さを記録
    ground_y_ = owner->GetTranslate().y;

    // ジャンプ初期上昇速度を設定
    current_jump_velocity_ = jump_power_;
    is_landing_            = false;

    float3 dir{0, 0, 0};
    if(Input::IsKeyDown(key_up_))
        dir += {0, 0, -1};
    if(Input::IsKeyDown(key_down_))
        dir += {0, 0, 1};
    if(Input::IsKeyDown(key_right_))
        dir += {-1, 0, 0};
    if(Input::IsKeyDown(key_left_))
        dir += {1, 0, 0};

    // 移動キーが押されていれば方向を固定、押されていなければ(0,0,0)で直上ジャンプ
    if((float)length(dir) > 0.0f) {
        jump_direction_ = normalize(dir);

        // 踏切時にモデルを飛ぶ方向に向ける
        if(auto mdl = owner->GetComponent<ComponentModel>()) {
            auto rot = quaternion::rotation_axis({0, 1, 0}, front_rot_ * DegToRad);
            mdl->SetRotationToVectorWithLimit(mul(jump_direction_, rot), rot_speed_);
        }
    }
    else {
        jump_direction_ = {0, 0, 0};
    }

    // アニメーション再生
    if(auto mdl = owner->GetComponent<ComponentModel>()) {
        if(is_holding_) {
            mdl->PlayAnimationNoSame("grab jump", false);
        }
        else {
            mdl->PlayAnimationNoSame("jump", false);
        }
    }
}

void ComponentStatePlayerJump::Update()
{
    __super::Update();

    auto owner = GetOwner();
    if(!owner)
        return;

    // 水平移動処理
    if((float)length(jump_direction_) > 0.0f) {
        owner->AddTranslate(jump_direction_ * air_move_speed_, true);
    }

    // Y軸移動処理
    owner->AddTranslate({0.0f, current_jump_velocity_, 0.0f}, true);

    float current_gravity = gravity_;
    if(std::abs(current_jump_velocity_) < 0.2f) {
        current_gravity *= 0.6f;
    }

    current_jump_velocity_ -= current_gravity;

    // 着地判定
    auto pos = owner->GetTranslate();
    if(pos.y <= ground_y_) {
        pos.y = ground_y_;
        owner->SetTranslate(pos);

        // 着地アニメーションを1回だけ再生
        if(!is_landing_) {
            is_landing_    = true;
            landing_Timer_ = 0.15f;    // 着地時の硬直(0.15)
            if(auto mdl = owner->GetComponent<ComponentModel>()) {
                mdl->PlayAnimationNoSame("land", false);
            }
        }

        // 着地硬直タイマーを減らす
        landing_Timer_ -= GetDeltaTime();    // フレーム時間分減らす

        if(landing_Timer_ <= 0.0f) {
            // タイマーが終わったら IdleWalk へ戻ります
            auto walk = owner->AddComponent<ComponentStateIdleWalk>();
            walk->SetKeys(key_up_, key_down_, key_left_, key_right_, key_jump_);
            walk->SetIsHolding(is_holding_);
            walk->SetFrontRotate(front_rot_);

            owner->RemoveComponent(shared_from_this());
            return;
        }
    }
}

ComponentStatePlayerJumpPtr ComponentStatePlayerJump::SetJumpPower(const float power)
{
    jump_power_ = power;
    return std::dynamic_pointer_cast<ComponentStatePlayerJump>(shared_from_this());
}

ComponentStatePlayerJumpPtr ComponentStatePlayerJump::SetGravity(const float gravity)
{
    gravity_ = gravity;
    return std::dynamic_pointer_cast<ComponentStatePlayerJump>(shared_from_this());
}

ComponentStatePlayerJumpPtr ComponentStatePlayerJump::SetAirMoveSpeed(const float speed)
{
    air_move_speed_ = speed;
    return std::dynamic_pointer_cast<ComponentStatePlayerJump>(shared_from_this());
}

ComponentStatePlayerJumpPtr ComponentStatePlayerJump::SetRotateSpeed(const float speed)
{
    rot_speed_ = speed;
    return std::dynamic_pointer_cast<ComponentStatePlayerJump>(shared_from_this());
}

float ComponentStatePlayerJump::GetJumpPower() const
{
    return jump_power_;
}

float ComponentStatePlayerJump::GetGravity() const
{
    return gravity_;
}

float ComponentStatePlayerJump::GetAirMoveSpeed() const
{
    return air_move_speed_;
}

ComponentStatePlayerJumpPtr ComponentStatePlayerJump::SetKeys(int up, int down, int left, int right, int jump)
{
    key_up_    = up;
    key_down_  = down;
    key_left_  = left;
    key_right_ = right;
    key_jump_  = jump;
    return std::dynamic_pointer_cast<ComponentStatePlayerJump>(shared_from_this());
}

void ComponentStatePlayerJump::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("State PlayerJump")) {
            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // 削除ボタン
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());

            // ジャンプパラメータ調整
            ImGui::DragFloat(u8"ジャンプ初速", &jump_power_, 0.05f);
            ImGui::DragFloat(u8"重力", &gravity_, 0.005f);
            ImGui::DragFloat(u8"空中移動速度", &air_move_speed_, 0.1f);
            ImGui::DragFloat(u8"移動回転角度", &rot_speed_, 1.0f);
            ImGui::DragFloat(u8"オブジェクト オフセット回転", &front_rot_, 1.0f);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentStatePlayerJump::SetIsHolding(bool hold)
{
    is_holding_ = hold;
}

CEREAL_REGISTER_TYPE(ComponentStatePlayerJump)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStatePlayerJump)
