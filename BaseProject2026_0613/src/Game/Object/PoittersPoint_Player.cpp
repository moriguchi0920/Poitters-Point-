//---------------------------------------------------------------------------
//! @file   PoittersPoint_Player.cpp
//! @brief  キャラクター(プレイヤー/CPU共通)
//---------------------------------------------------------------------------
#include "PoittersPoint_Player.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "Game/Component/ComponentStateMachine.h"
#include "Game/Component/ComponentGrabbable.h"
#include "Game/Component/ComponentHitPoints.h"
#include "Game/Component/ComponentCharacterPhysics.h"
#include "Game/Component/ComponentStateGrab.h"
#include "Game/Component/ComponentStateThrow.h"
#include "Game/Component/ComponentStateDown.h"
#include "Game/Component/ComponentStateDead.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {

bool Player::Init()
{
    Super::Init();

    SetName("Player");

    // 物理ボディは作成時の位置に置かれるため、先に位置を決めておく
    // (生成後に位置を変える時は PhysicsHelper::Warp を使う)
    SetTranslate({0, 8, 0});

    // アニメーションは SetCharacterStatus で設定する
    AddComponent<ComponentModel>(model_path_)->SetScaleAxisXYZ(local_scale_);

    // 当たり判定・重力・移動は ComponentPhysics(カプセル)に一本化
    AddComponent<ComponentCharacterPhysics>()->Setup(3.8f, 22.0f, ComponentPhysics::CollisionType::PLAYER);

    // 他者に掴まれて投げられるようにする
    AddComponent<ComponentGrabbable>()->SetLiftTime(0.5f);

    // 体力を管理するコンポーネントを追加
    AddComponent<ComponentHitPoints>()->SetMaxAndCurrentHP(5.0f);

    // ステートマシン(プレイヤー入力/CPU)は生成側のStageで付与する
    return true;
}

void Player::Update()
{
    Super::Update();

    // 脱落後は何もしない(倒れた姿勢は ComponentStateDown / ComponentStateDead が保つ)
    if(is_dead_) {
        return;
    }

    // HPが尽きていれば脱落処理
    UpdateDead();
    if(is_dead_) {
        return;
    }

    // 視線・ターゲット検出処理
    OnEyeSight();
}

void Player::UpdateDead()
{
    auto hp = GetComponent<ComponentHitPoints>();
    if(!hp || !hp->IsDead()) {
        return;
    }

    //-----------------------------------------------------------
    // 脱落を待つ条件
    // (HPが0になった時点で ComponentStateMachine が持ち物を落としてダウンさせているので、
    //  ここでは「安全に物理ボディを外せるか」だけを待つ)
    //-----------------------------------------------------------
    // 誰かに持ち上げられている/持たれている間は待つ(投げられて手から離れた後に脱落)
    if(auto grabbable = GetComponent<ComponentGrabbable>()) {
        auto state = grabbable->GetState();
        if(state == ComponentGrabbable::State::Reserved || state == ComponentGrabbable::State::Held) {
            return;
        }
    }
    // 自分が持ち上げ中/投げ中なら終わるまで待つ(途中で止めると物が手に残るため)
    if(GetComponent<ComponentStateGrab>() || GetComponent<ComponentStateThrow>()) {
        return;
    }
    // 吹き飛んでいる最中は着地まで待つ(空中で物理ボディを外すとその場で止まってしまう)
    if(auto mover = GetComponent<ComponentCharacterPhysics>()) {
        if(!mover->IsGrounded()) {
            return;
        }
    }

    StartDead();
}

void Player::StartDead()
{
    is_dead_ = true;

    // 持っている物はその場で手放し、操作/CPUのステートマシンを外す
    if(auto state_machine = GetComponent<ComponentStateMachine>()) {
        state_machine->DropGrabbingObject();
        RemoveComponent(state_machine);
    }

    // 歩き・ジャンプなどのステートを外す(外さないと入力やAIで動き続ける)
    // 脱落でダウンしている場合は、倒れた姿勢を保つためダウンだけ残す
    bool lying = false;
    for(auto& state : GetComponents<ComponentState>()) {
        if(std::dynamic_pointer_cast<ComponentStateDown>(state)) {
            lying = true;
            continue;
        }
        RemoveComponent(state);
    }

    // 脱落したキャラクターは掴めないようにする
    if(auto grabbable = GetComponent<ComponentGrabbable>()) {
        RemoveComponent(grabbable);
    }

    if(lying) {
        // 既に倒れているので、死体が他の物の邪魔をしないよう物理ボディだけ外す
        if(GetComponent<ComponentCharacterPhysics>())
            RemoveComponent<ComponentCharacterPhysics>();
        if(GetComponent<ComponentPhysics>())
            RemoveComponent<ComponentPhysics>();
    }
    else {
        // ダウンしていない(ステートマシンが無い等)場合は、モデルを倒して演出してから物理ボディを外す
        // (Playerには"dead"アニメーションが無いため SetUseAnimation(false))
        AddComponent<ComponentStateDead>()->SetUseAnimation(false);
    }
}

bool Player::IsAlive() const
{
    return !is_dead_;
}

void Player::OnEyeSight()
{
    auto state_machine = GetComponent<ComponentStateMachine>();
    auto mover         = GetComponent<ComponentCharacterPhysics>();

    float3 front = {0.0f, 0.0f, -1.0f};
    if(auto model = GetComponent<ComponentModel>()) {
        float3 v = -model->GetWorldVectorAxisZ();
        v.y      = 0.0f;
        if((float)length(v) > 0.0001f)
            front = normalize(v);
    }

    const float  cos_limit = cosf(grab_half_angle_ * DegToRad);
    const float3 center    = GetTranslate();

    // 掴めるのは「体が触れていて」「向いている方向にある」物だけ
    // (全オブジェクトを調べる必要がなく、触れている相手だけを見ればよい)
    // 物を優先し、物が無い時だけキャラクターを対象にする
    ObjectPtr nearest_item      = nullptr;
    ObjectPtr nearest_character = nullptr;
    float     item_distance     = FLT_MAX;
    float     character_dist    = FLT_MAX;

    if(mover) {
        for(auto& touch : mover->GetTouches()) {
            auto obj = touch.object_.lock();
            if(!obj)
                continue;

            auto grabbable = obj->GetComponent<ComponentGrabbable>();
            if(!grabbable || !grabbable->CanBeGrabbed())
                continue;

            // 接触点が正面の扇形の中にあるか(丸太の端に触れている時も、触れた場所で判定できる)
            float3 to_point = touch.point_ - center;
            to_point.y      = 0.0f;
            float distance  = (float)length(to_point);
            if(distance > 0.0001f && (float)dot(front, to_point / distance) < cos_limit)
                continue;

            if(std::dynamic_pointer_cast<Player>(obj)) {
                if(distance < character_dist) {
                    character_dist    = distance;
                    nearest_character = obj;
                }
            }
            else if(distance < item_distance) {
                item_distance = distance;
                nearest_item  = obj;
            }
        }
    }

    ObjectPtr target = nearest_item ? nearest_item : nearest_character;
    eyesight_target_ = target;

    // ステートマシンに掴み候補として登録する(触れていない・正面にない時は消す)
    // 人間はEキーでStartGrab、CPUは対象に触れた時にStartGrabする
    if(state_machine) {
        if(!target || !state_machine->GrabbableHit(target))
            state_machine->ClearGrabCandidate();
    }
}

const ObjectWeakPtr& Player::GetEyeSightTarget() const
{
    return eyesight_target_;
}

void Player::SetCharacterStatus(const CharacterData& data)
{
    // 代入
    model_path_      = data.model_path_;
    anims_           = data.anims_;
    local_scale_     = data.local_scale_;
    move_speed_      = data.move_speed_;
    throw_power_     = data.throw_power_;
    lift_time_ratio_ = data.lift_time_ratio_;
    // 入力されたアニメーションとスケールをモデルに反映する
    if(auto model = GetComponent<ComponentModel>()) {
        model->SetAnimation(anims_)->SetScaleAxisXYZ(local_scale_);
    }
}

float Player::GetMoveSpeed()
{
    return move_speed_;
}

}    // namespace PoittersPoint
