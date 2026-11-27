//---------------------------------------------------------------------------
//! @file   ComponentStateDown.cpp
//! @brief  ダウン状態コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentStateDown.h>
#include <Game/Component/ComponentCharacterPhysics.h>

void ComponentStateDown::Init()
{
    __super::Init();
    SetName<Component>("State Down");
}

void ComponentStateDown::Update()
{
    __super::Update();

    auto owner = GetOwner();
    auto mover = owner->GetComponent<ComponentCharacterPhysics>();
    auto model = owner->GetComponent<ComponentModel>();

    // AddComponent直後は Init が翌フレームになるため、初回 Update で元の姿勢を覚える
    if(!has_base_ && model) {
        base_matrix_ = model->GetMatrix();
        has_base_    = true;
        model->PlayAnimationNoSame("idle", true);
    }

    if(mover)
        mover->SetControlEnabled(false);

    const float dt          = GetDeltaTime();
    float       target_tilt = 90.0f;    // 仰向けに倒れる

    if(is_held_) {
        // 掴まれている間は倒れたまま
        air_time_   = 0.0f;
        lying_time_ = 0.0f;
    }
    else {
        air_time_           += dt;
        const bool grounded  = mover ? mover->IsGrounded() : true;
        // 吹き飛んだ直後の接地判定は無視する
        if((grounded && air_time_ > 0.15f) || air_time_ > max_air_time_)
            lying_time_ += dt;

        if(lying_time_ >= down_time_ && !is_eliminated_)
            target_tilt = 0.0f;    // 起き上がる(脱落したら起き上がらない)
    }

    // 傾きを目標へ近づける
    const float step = tilt_speed_ * dt;
    if(tilt_ < target_tilt)
        tilt_ = std::min(tilt_ + step, target_tilt);
    else
        tilt_ = std::max(tilt_ - step, target_tilt);
    ApplyTilt(tilt_);

    if(target_tilt <= 0.0f && tilt_ <= 0.0f)
        finished_ = true;
}

void ComponentStateDown::Exit()
{
    // モデルの姿勢と操作を元に戻す
    if(has_base_) {
        if(auto model = GetOwner()->GetComponent<ComponentModel>())
            model->SetMatrix(base_matrix_);
    }
    if(auto mover = GetOwner()->GetComponent<ComponentCharacterPhysics>())
        mover->SetControlEnabled(true);

    __super::Exit();
}

void ComponentStateDown::Restart()
{
    finished_   = false;
    air_time_   = 0.0f;
    lying_time_ = 0.0f;
}

void ComponentStateDown::RecoverSoon()
{
    is_held_    = false;
    lying_time_ = down_time_;
}

void ComponentStateDown::ApplyTilt(float degree)
{
    if(!has_base_)
        return;
    if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
        // 足元(モデル原点)を支点にモデルのローカルX軸で回す。正の角度で頭が後ろ(+Z)へ倒れる
        matrix tilt = cast(MGetRotX(degree * DegToRad));
        model->SetMatrix(mul(tilt, base_matrix_));
    }
}

void ComponentStateDown::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("State Down")) {
            ImGui::Text(u8"掴まれている: %s / 倒れている時間: %.2f", is_held_ ? u8"〇" : u8"×", lying_time_);
            ImGui::DragFloat(u8"ダウン時間", &down_time_, 0.05f, 0.0f, 10.0f);
            ImGui::DragFloat(u8"倒れる速さ(度/秒)", &tilt_speed_, 5.0f, 10.0f, 2000.0f);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentStateDown)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateDown)
