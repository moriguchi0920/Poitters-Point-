//---------------------------------------------------------------------------
//! @file   ComponentStateThrow.cpp
//! @brief  投げステート
//---------------------------------------------------------------------------
#include <Game/Component/ComponentStateThrow.h>
#include <Game/Component/ComponentStateMachine.h>
#include <Game/Object/PoittersPoint_Player.h>

void ComponentStateThrow::Init()
{
    __super::Init();

    SetName<Component>("State Throw");

    is_released_ = false;
    is_started_  = true;

    if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
        if(is_in_air_) {
            // 空中投げ：ThrowFast を上半身のみで再生
            model->PlayAnimationEx("throw_fast", "mixamorig:Spine", false);
        }
        else {
            // 地上投げ：通常版を全身で再生
            model->PlayAnimationNoSame("throw", false);
        }
    }
}

void ComponentStateThrow::Update()
{
    __super::Update();

    if(!is_started_)
        return;

    auto owner = GetOwner();
    auto model = owner->GetComponent<ComponentModel>();
    if(!model)
        return;

    // 空中投げ：アニメーションのルートモーション(Y)を打ち消す
    if(is_in_air_) {
        matrix mat        = model->GetMatrix();
        mat.translate().y = 0.0f;
        model->SetMatrix(mat);
    }

    // 手を離すタイミング(上半身の速い投げは早めに離す)
    // (手離し前にモーションが終わってしまった場合も、持ったまま固まらないよう離す)
    const float release_threshold = is_in_air_ ? 0.20f : 0.65f;
    if(!is_released_ && (release_threshold <= model->GetAnimationTime() || IsMotionEnd())) {
        if(auto state_machine = owner->GetComponent<ComponentStateMachine>())
            state_machine->ReleaseThrow(CalcThrowVelocity());
        is_released_ = true;
    }
}

float3 ComponentStateThrow::CalcThrowVelocity() const
{
    auto owner = GetOwner();

    // 正面ベクトル(モデルの -Z)。スケールやモーションの俯仰に左右されないよう水平成分だけ正規化する
    float3 forward = {0.0f, 0.0f, -1.0f};
    if(auto model = owner->GetComponent<ComponentModel>()) {
        float3 v = -model->GetWorldVectorAxisZ();
        v.y      = 0.0f;
        if((float)length(v) > 0.0001f)
            forward = normalize(v);
    }

    // キャラクターの投げる力
    float power = 1.0f;
    if(auto character = dynamic_cast<const PoittersPoint::Player*>(owner))
        power = character->GetThrowPower();

    return (forward * throw_speed_ + float3(0.0f, throw_up_speed_, 0.0f)) * power;
}

void ComponentStateThrow::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("State Throw")) {
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());

            ImGui::DragFloat(u8"投げる初速(正面)", &throw_speed_, 1.0f, 0.0f, 1000.0f, "%.1f");
            ImGui::DragFloat(u8"投げる初速(上)", &throw_up_speed_, 1.0f, 0.0f, 1000.0f, "%.1f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

bool ComponentStateThrow::GetIsFinished()
{
    // 手を離し、モーションも終わったら終了
    return is_released_ && IsMotionEnd();
}

bool ComponentStateThrow::IsMotionEnd() const
{
    // Init(モーション再生)前は終わっていない
    if(!is_started_)
        return false;

    // アニメーションが止まっているか(空中投げは上半身レイヤーの再生終了も待つ)
    if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
        return !model->IsPlaying() && !model->IsExPlaying("mixamorig:Spine");
    }
    return true;
}

CEREAL_REGISTER_TYPE(ComponentStateThrow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateThrow)
