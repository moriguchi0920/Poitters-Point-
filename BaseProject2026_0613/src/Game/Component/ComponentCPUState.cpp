#pragma once
#include <Game/Component/ComponentCPUState.h>
#include <Game/Component/ComponentStateTargetWalk.h>
#include <Game/Component/ComponentStateGrab.h>
#include <Game/Component/ComponentStateThrow.h>
#include <Game/Component/ComponentStatePlayerJump.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentHitPoints.h>
#include <Game/Component/ComponentPlayerState.h>
#include <Game/Object/PoittersPoint_Rock.h>
#include <Game/Object/PoittersPoint_Player.h>
#include <Game/Component/ComponentStateSetRangeWalk.h>
#include <Game/Component/ComponentStateThink.h>

#include <algorithm>
#include <random>

namespace {

//! CPU用の乱数(rand()は他の処理と共有されるため、CPUの行動用に別で持つ)
std::mt19937& CPURandom()
{
    static std::mt19937 engine(static_cast<unsigned int>(GetNowCount()));
    return engine;
}

}    // namespace

//---------------------------------------------------------------------------
// 性格ごとのパラメータ
//---------------------------------------------------------------------------
// clang-format off
const ComponentCPUState::PersonalityParam ComponentCPUState::PERSONALITY_PARAMS[] = {
    //  名前          思考最小  思考最大  避ける確率  投げる距離  キャラ掴み  うろうろ  近さ重視  HP重視  プレイヤー
    { "Aggressive",  0.20f,    0.70f,    0.25f,      45.0f,      0.50f,      0.15f,    1.20f,    0.3f,   10.0f },
    { "Cautious",    0.60f,    1.50f,    0.80f,      25.0f,      0.10f,      0.40f,    0.60f,    1.5f,    0.0f },
    { "Balanced",    0.40f,    1.00f,    0.50f,      35.0f,      0.25f,      0.25f,    1.00f,    0.8f,   20.0f },
};
// clang-format on

const ComponentCPUState::PersonalityParam& ComponentCPUState::Param() const
{
    return PERSONALITY_PARAMS[static_cast<int>(personality_)];
}

float ComponentCPUState::DistanceXZ(const float3& a, const float3& b)
{
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

float ComponentCPUState::RandomRange(float a, float b)
{
    std::uniform_real_distribution<float> dist(a, b);
    return dist(CPURandom());
}

//===========================================================================
// 初期化
//===========================================================================
void ComponentCPUState::Init()
{
    __super::Init();
    auto owner = GetOwner();

    can_throw_ = false;

    // 性格をランダムに決める
    personality_ = static_cast<CPU_PERSONALITY>(static_cast<int>(RandomRange(0.0f, static_cast<float>(CPU_PERSONALITY::NUM) - 0.001f)));

    // 思考はprevに入れないため、行動実績がない初期状態は
    // 手が空いているATTACK後と同じ基準で最初の行動を決めさせる
    prev_action_ = CPU_ACTION::ACTION_ATTACK;
    cur_action_  = CPU_ACTION::ACTION_THINK;
    // 全員が同時に動き出さないよう、最初の思考時間をばらけさせる
    owner->AddComponent<ComponentStateThink>()->SetThinkingFinishTime(RandomRange(0.2f, 1.2f));
}

//===========================================================================
// 更新
//===========================================================================
void ComponentCPUState::Update()
{
    __super::Update();

    auto owner = GetOwner();

    // ダウン中(吹き飛び中・誰かに掴まれている間も含む)は行動しない
    // 復帰した時は OnRecover() で考え直す
    if(IsDown()) {
        return;
    }

    float dt = GetDeltaTime();
    action_timer_ += dt;
    if(evasion_jump_cooldown_timer_ > 0.0f) {
        evasion_jump_cooldown_timer_ -= dt;
    }
    if(avoid_cooldown_ > 0.0f) {
        avoid_cooldown_ -= dt;
    }
    if(threat_check_timer_ > 0.0f) {
        threat_check_timer_ -= dt;
    }

    // 行動ごとの更新
    switch(cur_action_) {
    case CPU_ACTION::ACTION_THINK:
        {
            if(auto component_think = owner->GetComponent<ComponentStateThink>()) {
                if(component_think->GetFinished()) {
                    DecideAction();
                }
            }
            else {
                // 思考ステートが無い(他の処理でステートが変わった)場合は思考し直す
                StartThink();
            }
            break;
        }
    case CPU_ACTION::ACTION_GRAB:
        UpdateGrabAction();
        break;

    case CPU_ACTION::ACTION_ATTACK:
        UpdateAttackAction();
        break;

    case CPU_ACTION::ACTION_AVOID_ATTACKER:
    case CPU_ACTION::ACTION_WANDER:
        UpdateWalkEndAction();
        break;

    case CPU_ACTION::ACTION_EVASION_JUMP:
        {
            // ジャンプステートは着地後に自分で消えるため、消えたら考え直す
            if(!owner->GetComponent<ComponentStatePlayerJump>()) {
                StartThink(RandomRange(0.1f, 0.3f));
            }
            break;
        }
    default:
        break;
    }
}

//===========================================================================
// 掴みに行く行動の更新
//===========================================================================
void ComponentCPUState::UpdateGrabAction()
{
    auto owner = GetOwner();

    // 持ち上げが終わったら、少し間をおいて次の行動(投げに行く)を考える
    if(FinishGrab()) {
        StartThink(RandomRange(0.1f, 0.4f));
        return;
    }
    // 持ち上げ中は終わるまで待つ
    if(owner->GetComponent<ComponentStateGrab>()) {
        return;
    }

    auto component_walk = owner->GetComponent<ComponentStateWalkBase>();
    if(!component_walk) {
        StartThink();
        return;
    }

    // 掴みに行っている物が、誰かに取られた・投げられて飛んでいる・倒れた等で掴めなくなったら考え直す
    auto target = target_want_grab_.lock();
    if(!IsGrabTargetValid(target)) {
        target_want_grab_.reset();
        StartThink(RandomRange(0.1f, 0.4f));
        return;
    }

    // 時間がかかりすぎたら諦める(引っかかって近づけない時など)
    if(grab_timeout_ < action_timer_) {
        StartThink();
        return;
    }

    // 拾いに行く途中でも、狙われたら避ける
    if(TryInterruptByThreat()) {
        return;
    }

    // 体が触れて正面に来たら(=視線判定の掴み候補になったら)持ち上げる
    // 物理で体と物がぶつかるため、歩きは物に触れるまで進み続ける
    if(IsEyeSightTargetInReach() && StartGrab()) {
        return;
    }
    // 触れたまま止まってしまって掴めないなら考え直す
    if(component_walk->GetStopped()) {
        StartThink(RandomRange(0.2f, 0.5f));
    }
}

//===========================================================================
// 投げに行く行動の更新
//===========================================================================
void ComponentCPUState::UpdateAttackAction()
{
    auto owner = GetOwner();

    // 投げ終わったら手が空くので考え直す
    if(auto component_throw = owner->GetComponent<ComponentStateThrow>()) {
        if(component_throw->GetIsFinished()) {
            StartThink(RandomRange(0.1f, 0.5f));
        }
        return;
    }

    // 何らかの理由で手から物が無くなっていたら考え直す
    if(!IsHolding()) {
        StartThink();
        return;
    }

    // 狙っている相手が倒れた・誰かに持たれた場合は相手を選び直す
    auto target = attack_target_.lock();
    bool target_valid = false;
    if(auto character = std::dynamic_pointer_cast<PoittersPoint::Player>(target)) {
        target_valid = character->IsAlive();
        if(auto grabbable = character->GetComponent<ComponentGrabbable>()) {
            target_valid = target_valid && !grabbable->GetIsGrabbed();
        }
    }
    if(!target_valid) {
        auto new_target = ChooseAttackTarget();
        if(new_target.expired()) {
            // 相手がいなければうろうろする(持ったまま)
            StartWander();
        }
        else {
            StartAttackAction(new_target);
        }
        return;
    }

    float distance  = DistanceXZ(target->GetTranslate(), owner->GetTranslate());
    bool  in_range  = distance <= Param().throw_range;
    bool  is_facing = IsFacing(target, aim_angle_);

    // 投げられる距離で相手の方を向いたら投げる
    // 時間がかかりすぎた時は、向きが合っていなくても投げてしまう(持ったままうろうろし続けないように)
    if((in_range && is_facing) || attack_timeout_ < action_timer_) {
        // 低確率で、投げの代わりにその場で真上へジャンプして回避する
        if(IsEvadeJumpTrigger()) {
            StartEvadeJump();
            return;
        }
        StartThrow(false);
    }
}

//===========================================================================
// 避ける・うろうろする行動の更新
//===========================================================================
void ComponentCPUState::UpdateWalkEndAction()
{
    auto owner = GetOwner();

    auto component_walk = owner->GetComponent<ComponentStateWalkBase>();
    if(!component_walk) {
        StartThink();
        return;
    }

    // うろうろしている間に狙われたら避ける(避けている最中は除く)
    if(cur_action_ == CPU_ACTION::ACTION_WANDER && !IsHolding() && TryInterruptByThreat()) {
        return;
    }

    // 着いた・止まった・時間切れで考え直す
    if(component_walk->GetArrival() || component_walk->GetStopped() || walk_timeout_ < action_timer_) {
        // 避けた直後は素早く次の行動へ
        float think_time = (cur_action_ == CPU_ACTION::ACTION_AVOID_ATTACKER) ? RandomRange(0.1f, 0.3f) : -1.0f;
        StartThink(think_time);
    }
}

//===========================================================================
// 狙われていたら避ける
//===========================================================================
bool ComponentCPUState::TryInterruptByThreat()
{
    // 毎フレーム抽選すると必ず避けてしまうため、一定間隔で判定する
    if(0.0f < threat_check_timer_ || 0.0f < avoid_cooldown_) {
        return false;
    }
    threat_check_timer_ = 0.4f;

    auto threat = GetThreat();
    if(!threat) {
        return false;
    }
    // 行動の途中なので、思考後に判定する時よりは避けにくくする
    if(RandomRange(0.0f, 1.0f) < Param().avoid_chance * 0.6f) {
        StartAvoidAttacker(threat);
        return true;
    }
    return false;
}

//===========================================================================
// 次の行動を決める
//===========================================================================
void ComponentCPUState::DecideAction()
{
    //-----------------------------------------------------------
    // 物を持っている → 投げる相手を選んで投げに行く
    //-----------------------------------------------------------
    if(IsHolding()) {
        auto target = ChooseAttackTarget();
        if(!target.expired()) {
            StartAttackAction(target);
        }
        else {
            StartWander();
        }
        return;
    }

    //-----------------------------------------------------------
    // 狙われている → 性格に応じた確率で避ける(たまにジャンプで避ける)
    //-----------------------------------------------------------
    if(auto threat = GetThreat()) {
        if(avoid_cooldown_ <= 0.0f && RandomRange(0.0f, 1.0f) < Param().avoid_chance) {
            if(IsEvadeJumpTrigger()) {
                StartEvadeJump();
            }
            else {
                StartAvoidAttacker(threat);
            }
            return;
        }
    }

    //-----------------------------------------------------------
    // 手ぶら → 「物を拾う」「キャラクターを掴む」「うろうろ」を重み付きで抽選
    //-----------------------------------------------------------
    auto item      = ChooseGrabTarget(false);
    auto character = ChooseGrabTarget(true);

    float w_item      = item.expired() ? 0.0f : 1.0f;
    float w_character = character.expired() ? 0.0f : Param().grab_character_weight;
    // 拾える物が無い時はうろうろしやすく、連続でうろうろはしにくくする
    float w_wander = Param().wander_weight + (item.expired() ? 1.0f : 0.0f);
    if(prev_action_ == CPU_ACTION::ACTION_WANDER) {
        w_wander *= 0.3f;
    }

    float total = w_item + w_character + w_wander;
    float roll  = RandomRange(0.0f, total);

    if(roll < w_item) {
        StartGrabAction(item);
    }
    else if(roll < w_item + w_character) {
        StartGrabAction(character);
    }
    else {
        StartWander();
    }
}

//===========================================================================
// 行動の開始
//===========================================================================
void ComponentCPUState::StartThink(float duration)
{
    if(duration < 0.0f) {
        duration = RandomRange(Param().think_min, Param().think_max);
    }

    // 思考はprevに入れない
    if(cur_action_ != CPU_ACTION::ACTION_THINK) {
        prev_action_ = cur_action_;
    }
    cur_action_   = CPU_ACTION::ACTION_THINK;
    action_timer_ = 0.0f;

    if(auto think = ChangeState<ComponentStateThink>()) {
        think->SetThinkingFinishTime(think->GetCurThinkingTime() + duration);
        think->SetIsHolding(IsHolding());
    }
}

void ComponentCPUState::StartGrabAction(ObjectWeakPtr target)
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);
    if(!character_casted_owner || target.expired()) {
        StartThink();
        return;
    }
    target_want_grab_ = target;
    // 拾えるオブジェクトがあるところまで歩く
    if(auto target_walk = ChangeState<ComponentStateTargetWalk>()) {
        target_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
        target_walk->SetTargetPtr(target);
        // 当たり判定どうしがぶつかって中心間の距離は約9より縮まらないため、手の届く距離で「近くに来た」とする
        target_walk->SetNearbyDistance(grab_reach_);
    }

    cur_action_   = CPU_ACTION::ACTION_GRAB;
    action_timer_ = 0.0f;
}

void ComponentCPUState::StartAvoidAttacker(ObjectPtr attacker)
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);
    if(!character_casted_owner || !attacker) {
        StartThink();
        return;
    }

    //-----------------------------------------------------------
    // 避ける方向
    // 真後ろに逃げると投げられた物の進む方向と同じで当たりやすいため、
    // 「相手→自分」の向きに対して横(直角)方向を中心に、少し離れる向きへ逃げる
    //-----------------------------------------------------------
    float3 my_pos = owner->GetTranslate();
    float3 away   = my_pos - attacker->GetTranslate();
    away.y        = 0.0f;
    float len     = sqrtf(away.x * away.x + away.z * away.z);
    away          = (len < 0.001f) ? float3(0.0f, 0.0f, 1.0f) : away * (1.0f / len);

    // 横方向(左右どちらか)
    float3 side = {-away.z, 0.0f, away.x};
    // ステージの中心に近づく側を選びやすくする(端に追い詰められないように)。少しランダムも混ぜる
    float to_center = -(side.x * my_pos.x + side.z * my_pos.z);
    if(to_center + RandomRange(-30.0f, 30.0f) < 0.0f) {
        side = -side;
    }

    float3 dir      = side * 0.8f + away * 0.6f;
    float  dir_len  = sqrtf(dir.x * dir.x + dir.z * dir.z);
    dir             = dir * (1.0f / dir_len);
    float distance  = RandomRange(18.0f, 28.0f);

    // 逃げた先がステージの外側になるなら、反対の横方向へ逃げる
    float3 dest = my_pos + dir * distance;
    if(arena_radius_ < sqrtf(dest.x * dest.x + dest.z * dest.z)) {
        dir = -side * 0.8f + away * 0.6f;
        dir = dir * (1.0f / sqrtf(dir.x * dir.x + dir.z * dir.z));
    }

    // 指定した方向に指定した距離だけ歩く
    if(auto range_walk = ChangeState<ComponentStateSetRangeWalk>()) {
        range_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
        range_walk->SetWalkDirection(dir);
        range_walk->SetWalkDistance(distance);
    }

    avoid_cooldown_ = avoid_cooldown_time_;
    cur_action_     = CPU_ACTION::ACTION_AVOID_ATTACKER;
    action_timer_   = 0.0f;
}

void ComponentCPUState::StartAttackAction(ObjectWeakPtr target)
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);
    if(!character_casted_owner || target.expired()) {
        StartThink();
        return;
    }

    attack_target_ = target;

    // 相手の方へ歩く(投げる距離に入って相手の方を向いたらUpdateAttackActionで投げる)
    if(auto target_walk = ChangeState<ComponentStateTargetWalk>()) {
        target_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
        target_walk->SetTargetPtr(target);
        target_walk->SetIsHolding(true);
    }

    cur_action_   = CPU_ACTION::ACTION_ATTACK;
    action_timer_ = 0.0f;
}

void ComponentCPUState::StartWander()
{
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);
    if(!character_casted_owner) {
        return;
    }

    //-----------------------------------------------------------
    // ステージ内のランダムな地点へ歩く
    // 半径の平方根を取ると、円の中に均等に散らばる(取らないと中心に集まりすぎる)
    //-----------------------------------------------------------
    float  theta = RandomRange(0.0f, DX_TWO_PI_F);
    float  r     = sqrtf(RandomRange(0.0f, 1.0f)) * arena_radius_ * 0.8f;
    float3 dest  = {cosf(theta) * r, owner->GetTranslate().y, sinf(theta) * r};

    if(auto target_walk = ChangeState<ComponentStateTargetWalk>()) {
        target_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
        target_walk->SetTargetPos(dest);
        target_walk->SetNearbyDistance(3.0f);
        target_walk->SetIsHolding(IsHolding());
    }

    cur_action_   = CPU_ACTION::ACTION_WANDER;
    action_timer_ = 0.0f;
}

bool ComponentCPUState::IsEvadeJumpTrigger()
{
    auto owner = GetOwner();

    // クールダウン中はジャンプしない
    if(evasion_jump_cooldown_timer_ > 0.0f) {
        return false;
    }

    // 狙っている相手が近い時に脅威とみなす
    auto attacker = GetThreat();
    if(!attacker) {
        return false;
    }
    if(evasion_jump_threat_distance_ < DistanceXZ(attacker->GetTranslate(), owner->GetTranslate())) {
        return false;
    }

    // 頻発しないように低確率で抽選する
    return RandomRange(0.0f, 100.0f) < static_cast<float>(evasion_jump_chance_percent_);
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

    if(auto jump = ChangeState<ComponentStatePlayerJump>()) {
        jump->SetIsCPU(true);
        jump->SetIsHolding(IsHolding());
        jump->SetFrontRotate(front_rot);
    }

    evasion_jump_cooldown_timer_ = evasion_jump_cooldown_;
    cur_action_                  = CPU_ACTION::ACTION_EVASION_JUMP;
    action_timer_                = 0.0f;
}

//===========================================================================
// 周りの状況を調べる
//===========================================================================
bool ComponentCPUState::IsHolding() const
{
    // 持ち上げが終わって手に持っている(=投げられる)時だけ true
    // (基底の IsHolding() は持ち上げ中も含むため、CPU の判断では CanThrow() を使う)
    return CanThrow();
}

ObjectPtr ComponentCPUState::GetThreat()
{
    auto owner  = GetOwner();
    auto my_pos = owner->GetTranslate();

    ObjectPtr threat           = nullptr;
    float     nearest_distance = threat_distance_;

    for(auto& character : Scene::Object::GetArray<PoittersPoint::Player>()) {
        if(character.get() == owner || !character->IsAlive()) {
            continue;
        }
        // 物を持っている(持ち上げ中も含む)相手だけ
        auto state_machine = character->GetComponent<ComponentStateMachine>();
        if(!state_machine || !state_machine->GetGrabbing()) {
            continue;
        }
        float distance = DistanceXZ(character->GetTranslate(), my_pos);
        if(nearest_distance < distance) {
            continue;
        }
        // 相手がこちらを向いているか(相手の正面ベクトルと「相手→自分」の向きの角度)
        if(auto model = character->GetComponent<ComponentModel>()) {
            float3 forward = -model->GetWorldVectorAxisZ();
            float3 to_me   = my_pos - character->GetTranslate();
            forward.y      = 0.0f;
            to_me.y        = 0.0f;
            float f_len    = sqrtf(forward.x * forward.x + forward.z * forward.z);
            float t_len    = sqrtf(to_me.x * to_me.x + to_me.z * to_me.z);
            if(f_len < 0.001f || t_len < 0.001f) {
                continue;
            }
            // 単位ベクトル同士の内積 = cos(なす角)
            float cos_angle = (forward.x * to_me.x + forward.z * to_me.z) / (f_len * t_len);
            if(cos_angle < cosf(threat_angle_ * DegToRad)) {
                continue;
            }
        }
        nearest_distance = distance;
        threat           = character;
    }
    return threat;
}

ObjectWeakPtr ComponentCPUState::ChooseAttackTarget()
{
    auto owner  = GetOwner();
    auto my_pos = owner->GetTranslate();

    ObjectPtr best_target = nullptr;
    float     best_score  = -FLT_MAX;

    for(auto& character : Scene::Object::GetArray<PoittersPoint::Player>()) {
        if(character.get() == owner || !character->IsAlive()) {
            continue;
        }
        // 誰かに持たれている相手は狙わない
        if(auto grabbable = character->GetComponent<ComponentGrabbable>()) {
            if(grabbable->GetIsGrabbed()) {
                continue;
            }
        }

        //-----------------------------------------------------------
        // 点数をつけて一番高い相手を狙う
        //   近いほど高い / HPが減っているほど高い / 操作プレイヤーにはおまけ / ランダムで揺らす
        //-----------------------------------------------------------
        float distance = DistanceXZ(character->GetTranslate(), my_pos);
        float lost_hp  = 0.0f;
        if(auto hp = character->GetComponent<ComponentHitPoints>()) {
            lost_hp = hp->GetMaxHitPoints() - hp->GetHitPoints();
        }
        bool is_player = (character->GetComponent<ComponentPlayerState>() != nullptr);

        float score = -distance * Param().target_near_weight    //
                      + lost_hp * 10.0f * Param().target_weak_weight
                      + (is_player ? Param().target_player_bonus : 0.0f)
                      + RandomRange(0.0f, 15.0f);
        if(best_score < score) {
            best_score  = score;
            best_target = character;
        }
    }
    return std::weak_ptr(best_target);
}

ObjectWeakPtr ComponentCPUState::ChooseGrabTarget(bool character)
{
    auto owner  = GetOwner();
    auto my_pos = owner->GetTranslate();

    ObjectPtr best_target = nullptr;
    float     best_score  = -FLT_MAX;

    for(auto& obj : Scene::Object::GetArray<Object>()) {
        if(obj.get() == owner) {
            continue;
        }
        auto as_character = std::dynamic_pointer_cast<PoittersPoint::Player>(obj);
        if(character != (as_character != nullptr)) {
            continue;
        }
        if(!IsGrabTargetValid(obj)) {
            continue;
        }
        if(as_character) {
            // 物を持っている相手は掴みに行かない(先に投げられてしまうため)
            if(auto state_machine = as_character->GetComponent<ComponentStateMachine>()) {
                if(state_machine->GetGrabbing()) {
                    continue;
                }
            }
        }
        else {
            // 投げられて飛んでいる物は拾わない
            if(auto grabbable = obj->GetComponent<ComponentGrabbable>()) {
                if(grabbable->IsMoving()) {
                    continue;
                }
            }
        }

        float distance = DistanceXZ(obj->GetTranslate(), my_pos);
        // キャラクターは動き回って追いつきにくいため、近くにいる時だけ狙う
        float search_range = character ? 60.0f : grab_search_range_;
        if(search_range < distance) {
            continue;
        }

        // 近い物ほど選ばれやすく、ランダムで揺らして毎回同じ物に集まらないようにする
        float score = -distance + RandomRange(0.0f, 20.0f);
        if(best_score < score) {
            best_score  = score;
            best_target = obj;
        }
    }
    return std::weak_ptr(best_target);
}

bool ComponentCPUState::IsGrabTargetValid(const ObjectPtr& target) const
{
    if(!target) {
        return false;
    }
    auto grabbable = target->GetComponent<ComponentGrabbable>();
    // 置かれて止まっていて、誰も持ち上げ始めていない物だけ
    if(!grabbable || !grabbable->CanBeGrabbed()) {
        return false;
    }
    if(auto character = std::dynamic_pointer_cast<PoittersPoint::Player>(target)) {
        if(!character->IsAlive()) {
            return false;
        }
    }
    // 他のキャラクターが持ち上げ中なら掴めない
    for(auto& obj : Scene::Object::GetArray<PoittersPoint::Player>()) {
        if(obj.get() == GetOwner()) {
            continue;
        }
        if(auto state_machine = obj->GetComponent<ComponentStateMachine>()) {
            if(state_machine->IsGrabbingObject(target)) {
                return false;
            }
        }
    }
    return true;
}

bool ComponentCPUState::IsFacing(const ObjectPtr& target, float max_angle_deg) const
{
    auto owner = GetOwner();
    auto model = owner->GetComponent<ComponentModel>();
    if(!target || !model) {
        return true;
    }
    float3 forward = -model->GetWorldVectorAxisZ();
    float3 to_target = target->GetTranslate() - owner->GetTranslate();
    float  f_len     = sqrtf(forward.x * forward.x + forward.z * forward.z);
    float  t_len     = sqrtf(to_target.x * to_target.x + to_target.z * to_target.z);
    if(f_len < 0.001f || t_len < 0.001f) {
        return true;
    }
    // 単位ベクトル同士の内積 = cos(なす角)。cosが大きいほど角度は小さい
    float cos_angle = (forward.x * to_target.x + forward.z * to_target.z) / (f_len * t_len);
    return cosf(max_angle_deg * DegToRad) <= cos_angle;
}

bool ComponentCPUState::IsEyeSightTargetInReach() const
{
    // 掴み候補は「体が触れていて正面にある物」だけが登録される(Player::OnEyeSight)ため、
    // 候補があれば手が届いている。丸太のように長い物は中心までの距離が遠くても端に触れていれば掴める
    return IsGrabTargetExist();
}

//===========================================================================
// ダウンからの復帰
//===========================================================================
void ComponentCPUState::OnRecover()
{
    // ダウン中は持ち物を落とし、ステートも ComponentStateDown に置き換わっているため、
    // 手ぶらの状態として思考ステートから始め直す
    target_want_grab_.reset();
    StartThink(RandomRange(0.2f, 0.6f));
}

//===========================================================================
// GUI
//===========================================================================
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

            static const char* ACTION_NAMES[] = {"Think", "Grab", "Avoid", "Attack", "EvasionJump", "Wander"};
            ImGui::Text(u8"性格: %s", Param().name);
            ImGui::Text(u8"行動: %s (%.1f秒)", ACTION_NAMES[static_cast<int>(cur_action_)], action_timer_);
            ImGui::Text(u8"持っている: %s", IsHolding() ? "Yes" : "No");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentCPUState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCPUState)
