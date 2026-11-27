#include <Game/Component/ComponentCPUState.h>
#include <Game/Component/ComponentStateTargetWalk.h>
#include <Game/Component/ComponentStateGrab.h>
#include <Game/Component/ComponentStateThrow.h>
#include <Game/Component/ComponentStatePlayerJump.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Object/PoittersPoint_Rock.h>
#include <Game/Object/PoittersPoint_Player.h>
#include <Game/Component/ComponentStateSetRangeWalk.h>
#include <Game/Component/ComponentStateThink.h>

void ComponentCPUState::Init()
{
    __super::Init();
    auto owner = GetOwner();

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

    auto owner = GetOwner();

    // ダウン中(掴まれている間も含む)は行動しない
    if(IsDown())
        return;

    // 回避ジャンプのクールダウンを進める
    if(evasion_jump_cooldown_timer_ > 0.0f) {
        evasion_jump_cooldown_timer_ -= GetDeltaTime();
    }

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
                // 目的の物が誰かに取られた・動き出したら考え直す
                bool give_up = target_want_grab_.expired();
                if(auto target = target_want_grab_.lock()) {
                    if(auto grabbable = target->GetComponent<ComponentGrabbable>(); !grabbable || !grabbable->CanBeGrabbed())
                        give_up = true;
                }

                // 物に向かって歩き、体が触れて正面に来たら(=視線判定の候補になったら)持ち上げる
                // 引っかかって進めなくなったら考え直す
                if(give_up) {
                    target_want_grab_.reset();
                    StartThink();
                }
                else if(!StartGrab() && component_walk->GetStopped()) {
                    target_want_grab_.reset();
                    StartThink();
                }
            }
            if(FinishGrab()) {
                StartThink();
                if(auto component_think = owner->GetComponent<ComponentStateThink>()) {
                    component_think->SetIsHolding(IsHolding());
                }
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
    case CPU_ACTION::ACTION_EVASION_JUMP:
        {
            // ジャンプステートは着地後に自分で消えるため、消えたら考え直す
            if(!owner->GetComponent<ComponentStatePlayerJump>()) {
                StartThink();
            }
            break;
        }
    case CPU_ACTION::ACTION_ATTACK:
        {
            if(auto component_walk = owner->GetComponent<ComponentStateWalkBase>()) {
                if((component_walk->GetNearby() || component_walk->GetStopped()) && CanThrow()) {
                    // 低確率で、投げの代わりにその場で真上へジャンプして回避する
                    if(IsEvadeJumpTrigger()) {
                        StartEvadeJump();
                        break;
                    }
                    StartThrow(false);
                }
            }
            if(auto component_throw = owner->GetComponent<ComponentStateThrow>()) {
                if(component_throw->GetIsFinished()) {
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

void ComponentCPUState::OnRecover()
{
    // ダウンから復帰したら、手ぶらの状態として考え直す
    prev_action_ = CPU_ACTION::ACTION_ATTACK;
    cur_action_  = CPU_ACTION::ACTION_THINK;
    auto think   = ChangeState<ComponentStateThink>();
    think->SetIsHolding(IsHolding());
    think->SetThinkingFinishTime(think_time_);
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
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);
    if(!character_casted_owner || target.expired()) {
        return;
    }
    target_want_grab_ = target;
    // 拾えるオブジェクトがあるところまで歩く
    auto target_walk = ChangeState<ComponentStateTargetWalk>();
    target_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
    target_walk->SetTargetPtr(target);

    cur_action_ = CPU_ACTION::ACTION_GRAB;
}

void ComponentCPUState::StartAvoidAttacker()
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);
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
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);
    if(!character_casted_owner) {
        return;
    }

    // 最も近いキャラクターまで歩いて投げる
    auto target_walk = ChangeState<ComponentStateTargetWalk>();
    target_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
    target_walk->SetTargetPtr(GetNearestCharacter());
    target_walk->SetIsHolding(true);
    // 投げは離れた所から行う(物理で体や物がぶつかるため、密着するまで近づくと押し合いになって投げられない)
    target_walk->SetNearbyDistance(attack_range_);

    cur_action_ = CPU_ACTION::ACTION_ATTACK;
}

bool ComponentCPUState::IsEvadeJumpTrigger()
{
    auto owner = GetOwner();

    // クールダウン中はジャンプしない
    if(evasion_jump_cooldown_timer_ > 0.0f) {
        return false;
    }

    // 最も近い攻撃者(ものを持っているキャラクター)が近い時に脅威とみなす
    auto attacker = GetNearestAttacker().lock();
    if(!attacker) {
        return false;
    }
    float3 vec = attacker->GetTranslate() - owner->GetTranslate();
    float  dis = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    if(dis > evasion_jump_threat_distance_) {
        return false;
    }

    // 頻発しないように低確率で抽選する
    return (rand() % 100) < evasion_jump_chance_percent_;
}

void ComponentCPUState::StartEvadeJump()
{
    auto owner = GetOwner();
    if(owner->GetComponent<ComponentStatePlayerJump>()) {
        return;
    }

    // 歩きステートの向きをジャンプに引き継ぐ
    float front_rot = 0.0f;
    if(auto walk = owner->GetComponent<ComponentStateWalkBase>()) {
        front_rot = walk->GetFrontRotate();
    }

    auto jump = ChangeState<ComponentStatePlayerJump>();
    jump->SetIsCPU(true);
    jump->SetIsHolding(IsHolding());
    jump->SetFrontRotate(front_rot);

    evasion_jump_cooldown_timer_ = evasion_jump_cooldown_;
    cur_action_                  = CPU_ACTION::ACTION_EVASION_JUMP;
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
    case CPU_ACTION::ACTION_EVASION_JUMP:
        {
            if(GetGrabbing()) {
                // ジャンプ後もアイテムを持っているなら投げ直しに行く
                StartAttackAction();
            }
            else {
                // 投げた後と同じ動作
                // 近くにオブジェクトを持っている敵がいるなら逃げる
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
    auto characters = Scene::Object::GetArray<PoittersPoint::Player>();
    // 一番近いオブジェクトのポインタ
    ObjectPtr nearest_ptr = nullptr;
    // キャラクターへの距離を比べる用のfloat最大値(キャラクターの中から一番近いものを求めるため)
    float nearest_distance = FLT_MAX;
    for(auto& character : characters) {
        if(character == owner->SharedThis())
            continue;
        if(auto state_machine = character->GetComponent<ComponentStateMachine>(); state_machine && !state_machine->IsEliminated()) {
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
    auto characters = Scene::Object::GetArray<PoittersPoint::Player>();
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
        if(auto state_machine = character->GetComponent<ComponentStateMachine>(); state_machine && !state_machine->IsEliminated()) {
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
    // 掴めるオブジェクト(岩・丸太・スライムなど。キャラクターは除く)から一番近いものを探す
    auto      targets          = Scene::Object::GetArray<Object>();
    ObjectPtr target_ptr       = nullptr;
    float     nearest_distance = FLT_MAX;
    for(auto& target : targets) {
        if(target.get() == owner || std::dynamic_pointer_cast<PoittersPoint::Player>(target))
            continue;
        auto grabbable = target->GetComponent<ComponentGrabbable>();
        if(!grabbable || !grabbable->CanBeGrabbed())
            continue;

        float dis = (float)length(target->GetTranslate() - owner->GetTranslate());
        if(dis < nearest_distance) {
            nearest_distance = dis;
            target_ptr       = target;
        }
    }
    return std::weak_ptr(target_ptr);
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
