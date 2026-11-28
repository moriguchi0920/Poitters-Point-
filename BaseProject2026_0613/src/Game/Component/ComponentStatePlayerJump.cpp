//---------------------------------------------------------------------------
//! @file   ComponentStatePlayerJump.cpp
//! @brief  ジャンプステート(プレイヤー/CPU共通)
//---------------------------------------------------------------------------
#include <Game/Component/ComponentStatePlayerJump.h>
#include <Game/Component/ComponentStateControllerWalk.h>
#include <Game/Component/ComponentCharacterPhysics.h>
#include <Game/Object/PoittersPoint_Player.h>

void ComponentStatePlayerJump::Init()
{
    __super::Init();
    SetName<Component>("State PlayerJump");

    auto owner = GetOwner();
    if(!owner)
        return;

    is_jumped_  = false;
    is_landing_ = false;
    air_time_   = 0.0f;

    float3 dir{0, 0, 0};
    // CPU用ジャンプは入力キーを読まず直上ジャンプする
    // (Init は AddComponent の翌フレームに走るため、押された瞬間ではなく押しっぱなしで判定する)
    if(!is_cpu_) {
        if(IsKey(key_up_))
            dir += {0, 0, -1};
        if(IsKey(key_down_))
            dir += {0, 0, 1};
        if(IsKey(key_right_))
            dir += {-1, 0, 0};
        if(IsKey(key_left_))
            dir += {1, 0, 0};
    }

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

    // アニメーション再生（Init時に再生開始し、踏ん張りモーションから見せる）
    if(auto mdl = owner->GetComponent<ComponentModel>()) {
        mdl->PlayAnimation(is_holding_ ? "grab jump" : "jump", false);
    }
}

void ComponentStatePlayerJump::Update()
{
    __super::Update();

    auto owner = GetOwner();
    auto mover = owner->GetComponent<ComponentCharacterPhysics>();
    if(!mover) {
        // 物理移動が無いキャラクターはジャンプできない
        FinishJump();
        return;
    }

    const float dt = GetDeltaTime();

    // 踏ん張り（溜め）動作中は地上に留める
    if(startup_timer_ > 0.0f) {
        startup_timer_ -= dt;
        return;
    }

    // 踏み切り：上向きの初速を与える。以降の放物線は重力(物理)が描く
    if(!is_jumped_) {
        mover->Jump(jump_speed_);
        is_jumped_ = true;
        air_time_  = 0.0f;
    }

    air_time_ += dt;

    if(!is_landing_) {
        // 空中の水平移動(踏切時の方向で固定)
        if((float)length(jump_direction_) > 0.0f)
            mover->Move(jump_direction_ * air_move_speed_);

        // 着地判定(踏み切った直後の接地は無視する)
        if(air_time_ > 0.15f && mover->IsGrounded()) {
            is_landing_    = true;
            landing_Timer_ = 0.15f;    // 着地時の硬直
            if(auto mdl = owner->GetComponent<ComponentModel>()) {
                mdl->PlayAnimationNoSame(is_holding_ ? "grab idle" : "idle", false);
            }
        }
        return;
    }

    // 着地硬直
    landing_Timer_ -= dt;
    if(landing_Timer_ <= 0.0f)
        FinishJump();
}

void ComponentStatePlayerJump::FinishJump()
{
    auto owner = GetOwner();

    if(is_cpu_) {
        // CPUは歩きステートを自作しない(ComponentCPUState側でジャンプ消失を検知して再行動する)
        owner->RemoveComponent(shared_from_this());
        return;
    }

    // ControllerWalk へ戻る
    auto walk = owner->AddComponent<ComponentStateControllerWalk>();
    walk->SetKeys(key_up_, key_down_, key_left_, key_right_, key_jump_);
    walk->SetIsHolding(is_holding_);
    walk->SetFrontRotate(front_rot_);
    // キャラ定義の移動速度で歩き直す
    if(auto character = dynamic_cast<PoittersPoint::Player*>(owner)) {
        walk->SetMoveSpeed(character->GetMoveSpeed());
    }

    owner->RemoveComponent(shared_from_this());
}

ComponentStatePlayerJumpPtr ComponentStatePlayerJump::SetJumpSpeed(const float speed)
{
    jump_speed_ = speed;
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
            ImGui::DragFloat(u8"踏ん張り(溜め)時間", &startup_timer_, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat(u8"ジャンプ初速(上向き)", &jump_speed_, 1.0f, 0.0f, 1000.0f);
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

void ComponentStatePlayerJump::SetIsCPU(bool cpu)
{
    is_cpu_ = cpu;
}

CEREAL_REGISTER_TYPE(ComponentStatePlayerJump)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStatePlayerJump)
