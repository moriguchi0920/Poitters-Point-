#pragma once
#include <Game/Component/StateMachine/ComponentCPUState.h>
#include <Game/Component/State/ComponentStateTargetWalk.h>
#include <Game/Component/State/ComponentStateGrab.h>
#include <Game/Component/State/ComponentStateThrow.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Object/PoittersPoint_Rock.h>
#include <Game/Object/PoittersPoint_Character.h>
#include <Game/Component/State/ComponentStateSetRangeWalk.h>
#include <Game/Component/State/ComponentStateThink.h>

void ComponentCPUState::Init()
{
    __super::Init();
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Character*>(owner);



    can_grab_   = true;
    can_throw_  = false;
    prev_action_ = CPU_ACTION::ACTION_THINK;
    cur_action_ = CPU_ACTION::ACTION_THINK;
    owner->AddComponent<ComponentStateThink>()->SetThinkingFinishTime(1.5f);
}

void ComponentCPUState::Update()
{
    __super::Update();

    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Character*>(owner);

    // 行動ごとの更新
    switch(cur_action_) {
    case CPU_ACTION::ACTION_THINK:
        {
            if(auto component_think = owner->GetComponent<ComponentStateThink>()) {
                if(component_think->GetFinished()) {
                    ChangeActionFlexible();
                }
            }
            break;
        }

    case CPU_ACTION::ACTION_GRAB:
        {
            if(auto component_walk = owner->GetComponent<ComponentStateWalkBase>()) {
                if(!StartGrab()) {
                    StartThink();
                }
            }
            if(FinishGrab()) {
                StartThink();
            }
            break;
        }
    case CPU_ACTION::ACTION_AVOID_ATTACKER:
        {
            if(auto component_walk = owner->GetComponent<ComponentStateWalkBase>()) {
                if(component_walk->GetArrival() || component_walk->GetStopped()) {
                    StartThink();
                }
            }
            break;
        }
    case CPU_ACTION::ACTION_ATTACK:
        {
            if(auto component_walk = owner->GetComponent<ComponentStateWalkBase>()) {
                if((component_walk->GetArrival() || component_walk->GetStopped()) && can_throw_) {
                    ChangeState<ComponentStateThrow>()->SetThrowObject(grabbing_object_ptr_);
                    can_throw_ = false;
                }
            }
            if(auto component_throw = owner->GetComponent<ComponentStateThrow>()) {
                if(component_throw->GetIsFinished()) {
                    can_grab_ = true;
                    StartThink();
                }
            }
        }
    }
}

void ComponentCPUState::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("CPU State")) {
            //-------------------------------------------------------
            // 共通部分(共通化したい)

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分(SampleObjectController)を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            ImGui::TreePop();
        }
    }
    ImGui::End();
}


void ComponentCPUState::StartThink()
{
    prev_action_ = cur_action_;
    cur_action_ = CPU_ACTION::ACTION_THINK;
    ChangeState<ComponentStateThink>();
}

void ComponentCPUState::ChangeActionFlexible()
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Character*>(owner);
    switch (prev_action_)
    {
        case CPU_ACTION::ACTION_THINK:
        {
                break;
        }
        case CPU_ACTION::ACTION_GRAB:
        {
            if(character_casted_owner) {
                auto component_target_walk = ChangeState<ComponentStateTargetWalk>();
                component_target_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
                component_target_walk->SetTargetPtr(GetNearestCharacter());
            }
            cur_action_ = CPU_ACTION::ACTION_ATTACK;
            break;
        }
        case CPU_ACTION::ACTION_ATTACK:
        {

        }
    }
}

void ComponentCPUState::ChangeActionRandom()
{
}

ObjectPtr ComponentCPUState::GetNearestCharacter()
{
    auto owner = GetOwner();
    // キャラクターを配列で取得
    auto characters = Scene::Object::GetArray<PoittersPoint::Character>();
    // 一番近いオブジェクトのポインタ
    ObjectPtr nearest_ptr = nullptr;
    // キャラクターへの距離を比べる用のfloat最大値(キャラクターの中から一番近いものを求めるため)
    float nearest_distance = FLT_MAX;
    for(auto& character : characters) {
        if(character == owner->SharedThis())
            continue;
        if(auto state_machine = character->GetComponent<ComponentStateMachine>()) {
            float3 vec = character->GetTranslate() - owner->GetTranslate();
            float  dis = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
            if(dis < nearest_distance) {
                nearest_distance = dis;
                nearest_ptr      = character;
            }
        }
    }
    return nearest_ptr;
}

ObjectPtr ComponentCPUState::GetNearestAttacker()
{
    auto owner = GetOwner();
    // キャラクターを配列で取得
    auto characters = Scene::Object::GetArray<PoittersPoint::Character>();
    // 一番近いものを持っているキャラクターのポインタ
    ObjectPtr attacker_ptr = nullptr;
    // キャラクターへの距離を比べる用のfloat最大値(キャラクターの中から一番近いものを求めるため)
    float nearest_distance = FLT_MAX;
    // キャラクター配列を走査
    for(auto& character : characters) {
        // 自分ははじく
        if(std::static_pointer_cast<Object>(character) == owner->SharedThis())
            continue;
        // 対象のステートマシン取得
        if(auto state_machine = character->GetComponent<ComponentStateMachine>()) {
            // 座標を減算しベクトルを取得
            float3 vec = character->GetTranslate() - owner->GetTranslate();
            // ベクトルから距離を取得
            float dis = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
            // 距離を比較して最短距離であれば
            if(dis < nearest_distance) {
                // 最短距離を更新
                nearest_distance = dis;
                // ものを持っていたら
                if(state_machine->GetGrabbing()) {
                    // 攻撃者として登録
                    attacker_ptr = character;
                }
            }
        }
    }

    return attacker_ptr;
}

ObjectPtr ComponentCPUState::GetNearestGrabbableObj()
{
    auto owner = GetOwner();
    // ターゲットとなるオブジェクトを取得(ターゲットは仮で岩のみとする)
    auto      targets   = Scene::Object::GetArray<PoittersPoint::Rock>();
    ObjectPtr targetPtr = nullptr;
    // 最短距離
    float nearest_distance = FLT_MAX;
    // 岩オブジェクトを走査
    for(auto& target : targets) {
        // 距離を算出し比較、一番近い岩を目的地へ
        if(auto grabbable = target->GetComponent<ComponentGrabbable>()) {
            if(grabbable->GetIsGrabbed()) {
                continue;
            }
        }
        // 座標を減算しベクトルを取得
        float3 vec = target->GetTranslate() - owner->GetTranslate();
        // 距離取得
        float dis = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        // 比較して最短であれば
        if(dis < nearest_distance) {
            // 最短距離を更新
            nearest_distance = dis;
            // ターゲットとして登録
            targetPtr = target;
        }
    }
    return targetPtr;
}

bool ComponentCPUState::IsAttackerNearby()
{
    auto owner = GetOwner();
    if (auto obj = GetNearestAttacker())
    {
        float3 attacker_pos = obj->GetTranslate();
        float3 owner_pos    = owner->GetTranslate();
        float3 vec          = attacker_pos - owner_pos;
        float  dis          = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        if (dis < attacker_near_distance_)
        {
            return true;
        }
    }
    
    return false;
}

CEREAL_REGISTER_TYPE(ComponentCPUState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCPUState)
