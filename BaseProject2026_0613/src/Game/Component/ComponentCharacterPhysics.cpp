//---------------------------------------------------------------------------
//! @file   ComponentCharacterPhysics.cpp
//! @brief  キャラクター共通の物理移動コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentCharacterPhysics.h>
#include <Game/Component/ComponentGrabbable.h>
#include <System/Component/ComponentHitInfo.h>
#include <System/Physics/PhysicsEngine.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>

//============================================================================
// カプセル作成
//============================================================================
void ComponentCharacterPhysics::Setup(float radius, float height, ComponentPhysics::CollisionType type)
{
    auto owner = GetOwner();

    radius_ = std::max(radius, 0.1f);
    height_ = std::max(height, radius_ * 2.0f);

    // 物理の接触を ComponentPhysics 以外のコンポーネント(このクラスや Grabbable)にも通知する
    owner->SetStatus(Object::StatusBit::OnHitAllComponent, true);

    auto physics = owner->GetComponent<ComponentPhysics>();
    if(!physics)
        physics = owner->AddComponent<ComponentPhysics>();

    // 足元をオブジェクト原点にするため、カプセル中心を半分の高さへずらす(Create前に設定する)
    const float half_height = std::max((height_ - radius_ * 2.0f) * 0.5f, 0.1f);
    physics->SetTranslate({0.0f, height_ * 0.5f, 0.0f});
    physics->CreateCapsule(half_height, radius_);
    physics->SetCollisionType(type);
    physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Actived, true);
    physics->SetPhysicsReal(1.0f);    // 位置は速度だけで動かす(AddTranslateは使わない)
    physics->SetFriction(0.0f);       // 壁に張り付かないように摩擦0。減速はこのクラスで行う
    physics->SetRestitution(0.0f);

    // キャラクターは倒れない(向きはモデル側で表現している)
    physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::LockRotX, true);
    physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::LockRotY, true);
    physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::LockRotZ, true);
}

//============================================================================
// 物理前処理：移動要求を剛体の速度へ反映
//============================================================================
void ComponentCharacterPhysics::PrePhysics()
{
    __super::PrePhysics();

    if(Scene::IsPause())
        return;

    const float dt      = GetDeltaTime();
    time_since_ground_ += dt;

    // 古くなった接触を消す
    for(auto& touch : touches_)
        touch.elapsed_ += dt;
    std::erase_if(touches_, [this](const Touch& touch) { return touch.elapsed_ > touch_keep_time_ || touch.object_.expired(); });

    float3 request = move_velocity_;
    move_velocity_ = {0.0f, 0.0f, 0.0f};

    auto physics = GetPhysics();
    auto body    = physics ? physics->GetRigidBody() : nullptr;
    if(!body)
        return;

    // 掴まれている間は Grabbable が手の位置へ運ぶ(Kinematic)
    if(auto grabbable = GetOwner()->GetComponent<ComponentGrabbable>()) {
        if(grabbable->IsHeld())
            return;
    }

    // キャラクターは眠らせない(Jolt は止まったボディを眠らせ、眠ったボディは接触通知が来なくなるため
    // 立ち止まると接地判定が切れてしまう。キャラクターは数体なので常に起こしておいても負荷は小さい)
    physics::Engine::bodyInterface()->ActivateBody(JPH::BodyID((u32)body->bodyID()));

    float3 v = body->linearVelocity();

    // 万一 NaN・無限大が入った場合は止めて復帰する(Jolt に NaN を渡すとアサートで停止する)
    if(!std::isfinite((float)v.x) || !std::isfinite((float)v.y) || !std::isfinite((float)v.z))
        v = {0.0f, 0.0f, 0.0f};
    if(!std::isfinite((float)request.x) || !std::isfinite((float)request.z))
        request = {0.0f, 0.0f, 0.0f};

    if(control_enabled_) {
        // 水平は要求どおり、鉛直(重力・ジャンプ)は物理に任せる
        v.x = request.x;
        v.z = request.z;

        // 立ち止まっている間だけ摩擦を効かせ、わずかな斜面で滑らないようにする
        // (歩いている間は壁に張り付かないよう摩擦0)
        const bool standing = IsGrounded() && (float)length(request) < 0.001f;
        physics->SetFriction(standing ? 1.0f : 0.0f);
    }
    else {
        physics->SetFriction(0.0f);

        // 吹き飛び後の地面での滑り：指数的に減速
        if(IsGrounded()) {
            const float k  = expf(-ground_damping_ * dt);
            v.x           *= k;
            v.z           *= k;
        }
    }

    if(v.y < -max_fall_speed_)
        v.y = -max_fall_speed_;

    body->setLinearVelocity(v);
}

//============================================================================
// 接触通知：足元付近の接触なら接地
//============================================================================
void ComponentCharacterPhysics::OnHitComponent(const HitInfoPhysics& hit_info)
{
    if(!hit_info.collision_ || hit_info.collision_->GetOwner() != GetOwner())
        return;

    // 触れている相手を記録する(持ち上げの判定に使う)
    if(hit_info.hit_collision_) {
        if(Object* other = hit_info.hit_collision_->GetOwner(); other && other != GetOwner()) {
            auto it = std::find_if(touches_.begin(), touches_.end(), [other](const Touch& touch) { return touch.object_.lock().get() == other; });
            if(it == touches_.end())
                touches_.push_back({other->SharedThis(), hit_info.hit_position_, 0.0f});
            else
                *it = {it->object_, hit_info.hit_position_, 0.0f};
        }
    }

    // オブジェクト原点 = 足元。足元から半径の6割以内の高さの接触を「地面」とみなす
    // (地面メッシュ・岩の上・他キャラの頭の上、どれでも立てる)
    const float feet_y = GetOwner()->GetTranslate().y;
    if((float)hit_info.hit_position_.y <= feet_y + radius_ * 0.6f)
        time_since_ground_ = 0.0f;
}

//============================================================================
// 操作
//============================================================================
void ComponentCharacterPhysics::Move(const float3& velocity60)
{
    // AddTranslate(v) は 1/60秒あたり v 進むため、秒速に直すと 60倍
    move_velocity_ += float3(velocity60.x, 0.0f, velocity60.z) * 60.0f;
}

void ComponentCharacterPhysics::Jump(float speed)
{
    auto physics = GetPhysics();
    auto body    = physics ? physics->GetRigidBody() : nullptr;
    if(!body)
        return;

    float3 v = body->linearVelocity();
    v.y      = speed;
    body->setLinearVelocity(v);
    time_since_ground_ = ground_grace_time_ + 1.0f;    // 離陸した扱いにする
}

void ComponentCharacterPhysics::Launch(const float3& velocity)
{
    control_enabled_ = false;

    if(!std::isfinite((float)velocity.x) || !std::isfinite((float)velocity.y) || !std::isfinite((float)velocity.z))
        return;

    auto physics = GetPhysics();
    auto body    = physics ? physics->GetRigidBody() : nullptr;
    if(!body)
        return;

    body->setLinearVelocity(velocity);
    time_since_ground_ = ground_grace_time_ + 1.0f;
}

float3 ComponentCharacterPhysics::GetVelocity() const
{
    auto physics = GetPhysics();
    auto body    = physics ? physics->GetRigidBody() : nullptr;
    return body ? body->linearVelocity() : float3(0.0f, 0.0f, 0.0f);
}

//============================================================================
// GUI
//============================================================================
void ComponentCharacterPhysics::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("Character Physics")) {
            float3 v = GetVelocity();
            ImGui::Text(u8"速度: (%.1f, %.1f, %.1f)", (float)v.x, (float)v.y, (float)v.z);
            ImGui::Text(u8"接地: %s / 操作: %s", IsGrounded() ? u8"〇" : u8"×", control_enabled_ ? u8"可" : u8"不可");
            ImGui::DragFloat(u8"接地猶予(秒)", &ground_grace_time_, 0.01f, 0.0f, 0.5f);
            ImGui::DragFloat(u8"吹き飛び後の接地減速", &ground_damping_, 0.1f, 0.0f, 30.0f);
            ImGui::DragFloat(u8"落下速度上限", &max_fall_speed_, 1.0f, 10.0f, 2000.0f);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentCharacterPhysics)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCharacterPhysics)
