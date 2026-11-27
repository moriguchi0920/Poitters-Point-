//---------------------------------------------------------------------------
//! @file   ComponentGrabbable.cpp
//! @brief  掴み可能コンポーネント(投げ物・キャラクター共通)
//---------------------------------------------------------------------------
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentStateMachine.h>
#include <Game/Component/ComponentCharacterPhysics.h>
#include <Game/Component/ComponentHitPoints.h>
#include <System/Component/ComponentAttachModel.h>    // 掴み中の追従（手の位置）取得用
#include <System/Component/ComponentHitInfo.h>
#include <System/Physics/RigidBody.h>

namespace {
//! NaN・無限大が入っていないか
bool IsFinite(const float3& v)
{
    return std::isfinite((float)v.x) && std::isfinite((float)v.y) && std::isfinite((float)v.z);
}

//! 水平成分だけを正規化する(長さがほぼ0なら fallback)
float3 HorizontalDir(const float3& v, const float3& fallback)
{
    float3 h   = {v.x, 0.0f, v.z};
    float  len = (float)length(h);
    return len > 0.0001f ? h / len : fallback;
}
}    // namespace

//============================================================================
// 初期化処理
//============================================================================
void ComponentGrabbable::Init()
{
    __super::Init();

    // 物理の接触を ComponentPhysics 以外のコンポーネント(このクラス)にも通知してもらう
    GetOwner()->SetStatus(Object::StatusBit::OnHitAllComponent, true);
}

//============================================================================
// 更新処理：状態の後始末
//============================================================================
void ComponentGrabbable::Update()
{
    __super::Update();

    const float dt       = GetDeltaTime();
    time_since_support_ += dt;

    switch(state_) {
    case State::Free:
        // 置き物が止まったら、キャラクターに押されない Kinematic にする
        if(IsProp() && !is_resting_) {
            // 地面や他の置き物に支えられて止まった時だけ固定する
            // (キャラクターの頭の上などで固定すると、キャラクターが離れた時に宙に浮いてしまう)
            const bool supported = time_since_support_ < 0.25f;
            rest_time_           = (IsMoving() || !supported) ? 0.0f : rest_time_ + dt;
            if(rest_time_ >= rest_judge_time_)
                SetResting(true);
        }
        break;

    case State::Reserved:
        // 持ち上げていた人がいなくなった
        if(holder_.expired())
            CancelReserve();
        break;

    case State::Held:
        // 持っていた人がいなくなった
        if(holder_.expired())
            Drop();
        break;

    case State::Thrown:
        {
            thrown_time_ += dt;
            rest_time_    = IsMoving() ? 0.0f : rest_time_ + dt;

            // 止まった(または長時間たった)ら置かれた状態に戻す
            if(rest_time_ >= rest_judge_time_ || thrown_time_ > 10.0f) {
                state_   = State::Free;
                can_hit_ = false;
                if(auto physics = GetOwner()->GetComponent<ComponentPhysics>())
                    physics->SetIgnoreObject(nullptr);
                // (固定するかどうかは Free の処理で「支えられているか」を見て決める)
                rest_time_ = 0.0f;
            }
            break;
        }

    default:
        break;
    }
}

//============================================================================
// 物理シミュレーション前処理
//============================================================================
void ComponentGrabbable::PrePhysics()
{
    __super::PrePhysics();

    auto body = GetBody();
    if(!body)
        return;

    // 当たった瞬間の勢いとして、物理ステップ前の速度を覚えておく
    last_velocity_ = body->linearVelocity();

    // 手に持っている間は Kinematic のまま手の位置へ運ぶ(揺れなし)
    if(state_ == State::Held) {
        if(GetOwner()->GetComponent<ComponentAttachModel>()) {
            // 見た目(オブジェクト)は AttachModel が手のボーン行列に置く。
            // 物理ボディも同じ姿勢(+形状の中心オフセット)へ直接置き、見た目と当たり判定を一致させる。
            // (MoveKinematic は「このフレームの経過時間で着く速度」を与えるが、物理は1/60秒固定で
            //  1フレームに0～4回進むため、速度が何倍にも効いて行き過ぎ・振動・巨大な速度の原因になる。
            //  持っている間は当たり判定を切っているので、速度を使わず直接置いてよい)
            matrix     target = CalcHeldBodyMatrix();
            float3     pos    = target.translate();
            quaternion rot    = normalize(quaternion(float3x3(target)));
            if(IsFinite(pos) && std::isfinite((float)rot.x) && std::isfinite((float)rot.w)) {
                body->setLinearVelocity({0.0f, 0.0f, 0.0f});
                body->setAngularVelocity({0.0f, 0.0f, 0.0f});
                body->setPosition(pos, false);
                body->setRotation(rot, false);
            }
        }
    }
}

//============================================================================
// 物理の接触通知：投げ中のヒット判定
//============================================================================
void ComponentGrabbable::OnHitComponent(const HitInfoPhysics& hit_info)
{
    // 自分のボディの接触だけを見る
    if(!hit_info.collision_ || hit_info.collision_->GetOwner() != GetOwner())
        return;

    Object* target = hit_info.hit_collision_ ? hit_info.hit_collision_->GetOwner() : nullptr;
    if(!target)
        return;

    // 地面・壁・固定済みの置き物に触れている = 支えられている
    if(auto type = hit_info.hit_collision_->GetCollisionType();
       type == ComponentPhysics::CollisionType::GROUND || type == ComponentPhysics::CollisionType::WALL) {
        time_since_support_ = 0.0f;
    }
    else if(auto other = target->GetComponent<ComponentGrabbable>(); other && other->is_resting_) {
        time_since_support_ = 0.0f;
    }

    // 置かれて止まっている物に、投げられた物が当たったら弾き飛ばされる
    if(is_resting_ && state_ == State::Free) {
        if(auto other = target->GetComponent<ComponentGrabbable>(); other && other->IsThrown()) {
            SetResting(false);
            if(auto body = GetBody())
                body->setLinearVelocity(other->last_velocity_ * 0.6f + float3(0.0f, 20.0f, 0.0f));
        }
        return;
    }

    if(state_ != State::Thrown || !can_hit_)
        return;

    // 投げた本人には当たらない
    if(auto thrower = thrower_.lock(); thrower && thrower.get() == target)
        return;

    if(OnImpactFunc)
        OnImpactFunc(target);

    HitTarget(target);
}

void ComponentGrabbable::HitTarget(Object* target)
{
    // HPを持たない相手(地面・壁・他の物)は物理の跳ね返りだけ
    auto hp = target->GetComponent<ComponentHitPoints>();
    if(!hp)
        return;

    // 同じ投げで同じ相手に連続ヒットさせない
    if(IsAlreadyHit(target))
        return;
    hit_targets_.push_back(target->SharedThis());

    // 転がり終わりのような弱い当たりは無効
    if((float)length(last_velocity_) < move_speed_threshold_)
        return;

    hp->TakeDamage(damage_);

    // 吹き飛ぶ方向：投げ物の進行方向と、当たった位置関係(投げ物→相手)の中間
    float3 travel = HorizontalDir(last_velocity_, {0.0f, 0.0f, 1.0f});
    float3 away   = HorizontalDir(target->GetTranslate() - GetOwner()->GetTranslate(), travel);
    float3 dir    = HorizontalDir(travel + away, travel);

    float3 knock = dir * knockback_power_ + float3(0.0f, knockback_up_, 0.0f);

    if(auto state_machine = target->GetComponent<ComponentStateMachine>()) {
        // キャラクター：持ち物を落としてダウン
        state_machine->KnockDown(knock);
    }
    else if(auto mover = target->GetComponent<ComponentCharacterPhysics>()) {
        mover->Launch(knock);
    }
}

bool ComponentGrabbable::IsAlreadyHit(const Object* target) const
{
    for(auto& weak : hit_targets_) {
        if(auto hit = weak.lock(); hit && hit.get() == target)
            return true;
    }
    return false;
}

//============================================================================
// 状態の取得
//============================================================================
bool ComponentGrabbable::CanBeGrabbed() const
{
    if(state_ != State::Free)
        return false;

    auto owner = GetOwner();

    // キャラクター：モーション中・物を持っている・空中は掴めない
    if(auto state_machine = owner->GetComponent<ComponentStateMachine>()) {
        if(state_machine->IsBusy())
            return false;
        if(auto mover = owner->GetComponent<ComponentCharacterPhysics>()) {
            if(!mover->IsGrounded())
                return false;
        }
        return true;
    }

    // 物：転がっている・飛んでいる最中は掴めない
    return !IsMoving();
}

bool ComponentGrabbable::IsMoving() const
{
    auto body = GetBody();
    if(!body)
        return false;
    return (float)length(body->linearVelocity()) > move_speed_threshold_;
}

bool ComponentGrabbable::IsThrower(const ObjectPtr& target) const
{
    return target && thrower_.lock() == target;
}

//============================================================================
// 操作
//============================================================================
bool ComponentGrabbable::Reserve(const ObjectPtr& lifter)
{
    if(!lifter || lifter.get() == GetOwner() || !CanBeGrabbed())
        return false;

    state_  = State::Reserved;
    holder_ = lifter;
    rest_time_ = 0.0f;

    // 持ち上げられ始めたキャラクターは動けなくなる
    if(auto state_machine = GetOwner()->GetComponent<ComponentStateMachine>())
        state_machine->OnGrabbed();

    return true;
}

void ComponentGrabbable::CancelReserve()
{
    if(state_ != State::Reserved)
        return;

    state_ = State::Free;
    holder_.reset();

    if(auto state_machine = GetOwner()->GetComponent<ComponentStateMachine>())
        state_machine->OnReleased(false);
}

void ComponentGrabbable::Hold(const ObjectPtr& holder, std::string_view bone)
{
    // 予約した本人だけが持てる
    if(state_ != State::Reserved || !holder || holder_.lock() != holder)
        return;

    state_ = State::Held;

    auto owner = GetOwner();
    if(auto physics = owner->GetComponent<ComponentPhysics>()) {
        // 持っている間は誰ともぶつからない(手の位置へ Kinematic で運ぶ)
        physics->SetIgnoreObject(holder.get());
        saved_overlap_ = physics->GetOverlapCollision();
        physics->SetOverlapCollision(0xffff);
        if(auto body = physics->GetRigidBody()) {
            body->setMotionType(physics::MotionType::Kinematic, true);
            body->setLinearVelocity({0.0f, 0.0f, 0.0f});
            body->setAngularVelocity({0.0f, 0.0f, 0.0f});
        }
    }

    is_resting_ = false;

    // 名前ではなくポインタでくっつける(同名のキャラクターがいても取り違えない)
    if(owner->GetComponent<ComponentAttachModel>())
        owner->RemoveComponent<ComponentAttachModel>();
    owner->AddComponent<ComponentAttachModel>()->SetAttachObject(holder, bone);
}

matrix ComponentGrabbable::CalcHeldBodyMatrix() const
{
    auto owner   = GetOwner();
    auto attach  = owner->GetComponent<ComponentAttachModel>();
    auto physics = owner->GetComponent<ComponentPhysics>();

    matrix hand = attach ? attach->GetPutOnMatrix() : owner->GetMatrix();

    // ボーン行列にはモデルのスケールが入っているため、回転だけを取り出す(物理ボディはスケールを持てない)
    float3 ax = normalize(hand._11_12_13);
    float3 ay = normalize(hand._21_22_23);
    float3 az = normalize(cross(ax, ay));
    ay        = cross(az, ax);

    // キャラクターは回転ロック(拘束)があるため、ボディは回さず位置だけ合わせる
    // (回したまま Dynamic に戻すと、拘束が元の向きへ無理に戻そうとして暴れる)
    if(!IsProp()) {
        ax = {1.0f, 0.0f, 0.0f};
        ay = {0.0f, 1.0f, 0.0f};
        az = {0.0f, 0.0f, 1.0f};
    }
    hand._11_12_13 = ax;
    hand._21_22_23 = ay;
    hand._31_32_33 = az;

    // ボディ = 形状の中心オフセット(physics_transform) × オブジェクトの行列
    return physics ? mul(physics->GetMatrix(), hand) : hand;
}

bool ComponentGrabbable::IsProp() const
{
    return GetOwner()->GetComponent<ComponentStateMachine>() == nullptr;
}

void ComponentGrabbable::SetResting(bool resting)
{
    auto body = GetBody();
    if(!body) {
        is_resting_ = false;
        return;
    }

    is_resting_ = resting;
    rest_time_  = 0.0f;
    if(resting) {
        body->setLinearVelocity({0.0f, 0.0f, 0.0f});
        body->setAngularVelocity({0.0f, 0.0f, 0.0f});
        body->setMotionType(physics::MotionType::Kinematic, false);
    }
    else {
        body->setMotionType(physics::MotionType::Dynamic, true);
    }
}

void ComponentGrabbable::ReleaseFromHand()
{
    auto owner = GetOwner();
    if(owner->GetComponent<ComponentAttachModel>())
        owner->RemoveComponent<ComponentAttachModel>();

    if(auto physics = owner->GetComponent<ComponentPhysics>()) {
        physics->SetOverlapCollision(saved_overlap_);
        if(auto body = physics->GetRigidBody())
            body->setMotionType(physics::MotionType::Dynamic, true);
    }
    is_resting_ = false;
    rest_time_  = 0.0f;
}

void ComponentGrabbable::Throw(const ObjectPtr& thrower, const float3& velocity)
{
    if(state_ != State::Held)
        return;

    ReleaseFromHand();

    state_       = State::Thrown;
    can_hit_     = true;
    thrower_     = thrower;
    rest_time_   = 0.0f;
    thrown_time_ = 0.0f;
    holder_.reset();
    hit_targets_.clear();

    auto physics = GetOwner()->GetComponent<ComponentPhysics>();
    auto body    = physics ? physics->GetRigidBody() : nullptr;
    if(body) {
        // 投げた本人とは止まるまでぶつからない(手元で押し合わないように)
        physics->SetIgnoreObject(thrower.get());

        float3 v = velocity * throw_speed_scale_;
        if(!IsFinite(v))
            v = {0.0f, 0.0f, 0.0f};
        body->setLinearVelocity(v);
        last_velocity_ = v;

        float3 fwd = HorizontalDir(v, {0.0f, 0.0f, 0.0f});
        if((float)length(fwd) > 0.0f) {
            const float3 up   = {0.0f, 1.0f, 0.0f};
            const float3 side = normalize(cross(up, fwd));

            // 円柱(丸太)は軸を進行方向と直交させ、そのまま転がれる向きにする
            if(align_roll_axis_) {
                matrix m    = matrix::identity();
                m._11_12_13 = fwd;
                m._21_22_23 = side;    // ボディY軸 = 円柱の軸
                m._31_32_33 = cross(fwd, side);
                body->setRotation(normalize(quaternion(float3x3(m))));
            }

            // 地面を転がる回転(ω = up × v / r)を与えると、着地後に自然に転がる
            if(roll_spin_ > 0.0f) {
                const float  radius = std::max(roll_radius_, 0.1f);
                const float3 hv     = {v.x, 0.0f, v.z};
                body->setAngularVelocity(cross(up, hv) / radius * roll_spin_);
            }
        }
    }

    // 投げられたキャラクターは着地後ダウン
    if(auto state_machine = GetOwner()->GetComponent<ComponentStateMachine>())
        state_machine->OnReleased(true);
}

void ComponentGrabbable::Drop()
{
    if(state_ == State::Reserved) {
        CancelReserve();
        return;
    }
    if(state_ != State::Held)
        return;

    auto holder = holder_.lock();
    ReleaseFromHand();

    // ダメージなしで落下させる(止まるまでは持っていた人とぶつからない)
    state_       = State::Thrown;
    can_hit_     = false;
    thrower_     = holder;
    rest_time_   = 0.0f;
    thrown_time_ = 0.0f;
    holder_.reset();

    if(auto physics = GetOwner()->GetComponent<ComponentPhysics>()) {
        physics->SetIgnoreObject(holder.get());
        if(auto body = physics->GetRigidBody())
            body->setLinearVelocity({0.0f, 0.0f, 0.0f});
    }

    if(auto state_machine = GetOwner()->GetComponent<ComponentStateMachine>())
        state_machine->OnReleased(true);
}

std::shared_ptr<physics::RigidBody> ComponentGrabbable::GetBody() const
{
    // ComponentPhysics の Init は遅れて走り、GUIで作り直されることもあるため毎回取得する
    if(auto physics = GetOwner()->GetComponent<ComponentPhysics>())
        return physics->GetRigidBody();
    return nullptr;
}

//============================================================================
// GUI処理
//============================================================================
void ComponentGrabbable::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("Grabbable")) {
            constexpr const char* state_names[] = {u8"置かれている", u8"持ち上げ中", u8"持たれている", u8"飛んでいる"};
            ImGui::Text(u8"状態: %s / 掴める: %s", state_names[(int)state_], CanBeGrabbed() ? u8"〇" : u8"×");

            ImGui::DragFloat(u8"持ち上げ時間", &lift_time_, 0.05f, 0.0f, 5.0f);
            ImGui::DragFloat(u8"ダメージ", &damage_, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat(u8"吹き飛ばす強さ", &knockback_power_, 1.0f, 0.0f, 1000.0f);
            ImGui::DragFloat(u8"浮かせる強さ", &knockback_up_, 1.0f, 0.0f, 1000.0f);
            ImGui::DragFloat(u8"投げる速さ倍率", &throw_speed_scale_, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat(u8"転がり回転", &roll_spin_, 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat(u8"移動中しきい値", &move_speed_threshold_, 1.0f, 1.0f, 200.0f, "%.1f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentGrabbable)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentGrabbable)
