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

    can_grab_  = true;
    can_throw_ = false;
    // 思考はprevに入れないため、行動実績がない初期状態は
    // 手が空いているATTACK後と同じ基準で最初の行動を決めさせる
    prev_action_ = CPU_ACTION::ACTION_ATTACK;
    cur_action_  = CPU_ACTION::ACTION_THINK;
    owner->AddComponent<ComponentStateThink>()->SetThinkingFinishTime(think_time_);
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
                // 掴み対象に当たって掴める状態になるまで歩き続ける
                // 掴めずにたどり着けない/止まってしまったら考え直す
                if((component_walk->GetArrival() || component_walk->GetStopped())) {
                    if(!StartGrab())
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
    // 思考はprevに入れない
    if(cur_action_ != CPU_ACTION::ACTION_THINK) {
        prev_action_ = cur_action_;
        cur_action_  = CPU_ACTION::ACTION_THINK;
        auto think   = ChangeState<ComponentStateThink>();
        think->SetThinkingFinishTime(think->GetCurThinkingTime() + think_time_);
    }
}

void ComponentCPUState::RestartThink()
{
    // 行動中の考え直しはStartThinkだとprevに記録してしまうため、思考時間だけ延長する
    if(auto think = GetOwner()->GetComponent<ComponentStateThink>()) {
        think->SetThinkingFinishTime(think->GetCurThinkingTime() + think_time_);
    }
}

void ComponentCPUState::StartGrabAction(ObjectWeakPtr target)
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Character*>(owner);
    if(!character_casted_owner || target.expired()) {
        return;
    }

    // 拾えるオブジェクトがあるところまで歩く
    auto target_walk = ChangeState<ComponentStateTargetWalk>();
    target_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
    target_walk->SetTargetPtr(target);

    cur_action_ = CPU_ACTION::ACTION_GRAB;
}

void ComponentCPUState::StartAvoidAttacker()
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Character*>(owner);
    // ものを持っている最も近いキャラクターを取得
    auto attacker = GetNearestAttacker();
    if(!character_casted_owner || attacker.expired()) {
        return;
    }

    // 攻撃者から離れる方向に逃げる
    float3 dir    = owner->GetTranslate() - attacker.lock()->GetTranslate();
    dir.y         = 0.0f;
    float dir_len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if(dir_len < 0.001f) {
        dir = {0.0f, 0.0f, 1.0f};
    }
    else {
        dir = dir * (1.0f / dir_len);
    }

    // 攻撃者から離れる距離だけ歩く
    auto range_walk = ChangeState<ComponentStateSetRangeWalk>();
    range_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
    range_walk->SetWalkDirection(dir);
    range_walk->SetWalkDistance(escape_offset_);

    cur_action_ = CPU_ACTION::ACTION_AVOID_ATTACKER;
}

void ComponentCPUState::StartAttackAction()
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Character*>(owner);
    if(!character_casted_owner) {
        return;
    }

    // 最も近いキャラクターまで歩いて投げる
    auto target_walk = ChangeState<ComponentStateTargetWalk>();
    target_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
    target_walk->SetTargetPtr(GetNearestCharacter());

    cur_action_ = CPU_ACTION::ACTION_ATTACK;
}

void ComponentCPUState::ChangeActionFlexible()
{
    // 思考はprevに入れないため、このcaseに来ることはない(安全のため何もしない)
    if(prev_action_ == CPU_ACTION::ACTION_THINK) {
        return;
    }

    // 直前の行動ごとに次の行動を決める
    // ACTIONを追加する場合は、caseとその行動を開始する関数を増やすだけで対応できる
    switch(prev_action_) {
    case CPU_ACTION::ACTION_ATTACK:
        {
            // 投げた後は手が空いているため、近くにオブジェクトを持っている敵がいるなら逃げる
            if(IsAttackerNearby()) {
                StartAvoidAttacker();
            }
            // 拾えるオブジェクトが近所にあるなら拾いに行く
            else if(auto target = GetNearestGrabbableObj().lock()) {
                StartGrabAction(target);
            }
            else {
                // どちらもない場合は考え直す
                RestartThink();
            }
            break;
        }
    case CPU_ACTION::ACTION_AVOID_ATTACKER:
        {
            // 逃げた後は基本的にオブジェクトを拾いに行く
            if(auto target = GetNearestGrabbableObj().lock()) {
                StartGrabAction(target);
            }
            else {
                RestartThink();
            }
            break;
        }
    case CPU_ACTION::ACTION_GRAB:
        {
            if(GetGrabbing()) {
                // 拾った後は敵にものを投げに行く
                StartAttackAction();
            }
            else {
                // 拾えなかったので攻撃後と同じ動作
                // 投げた後は手が空いているため、近くにオブジェクトを持っている敵がいるなら逃げる
                if(IsAttackerNearby()) {
                    StartAvoidAttacker();
                }
                // 拾えるオブジェクトが近所にあるなら拾いに行く
                else if(auto target = GetNearestGrabbableObj().lock()) {
                    StartGrabAction(target);
                }
                else {
                    // どちらもない場合は考え直す
                    RestartThink();
                }
            }
            break;
        }
    }
}

void ComponentCPUState::ChangeActionRandom()
{
}

ObjectWeakPtr ComponentCPUState::GetNearestCharacter()
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
    return std::weak_ptr(nearest_ptr);
}

ObjectWeakPtr ComponentCPUState::GetNearestAttacker()
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
            // ものを持っている最も近いキャラクターを攻撃者とする
            if(state_machine->GetGrabbing() && dis < nearest_distance) {
                // 最短距離を更新
                nearest_distance = dis;
                // 攻撃者として登録
                attacker_ptr = character;
            }
        }
    }

    return std::weak_ptr(attacker_ptr);
}

ObjectWeakPtr ComponentCPUState::GetNearestGrabbableObj()
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
    return std::weak_ptr(targetPtr);
}

bool ComponentCPUState::IsAttackerNearby()
{
    auto owner = GetOwner();
    if(auto obj = GetNearestAttacker().lock()) {
        float3 attacker_pos = obj->GetTranslate();
        float3 owner_pos    = owner->GetTranslate();
        float3 vec          = attacker_pos - owner_pos;
        float  dis          = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        if(dis < attacker_near_distance_) {
            return true;
        }
    }

    return false;
}

CEREAL_REGISTER_TYPE(ComponentCPUState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCPUState)
