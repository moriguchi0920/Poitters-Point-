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

    // 視線・ターゲット検出処理
    OnEyeSight();
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
