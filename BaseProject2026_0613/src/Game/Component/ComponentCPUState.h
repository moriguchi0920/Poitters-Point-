#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentStateMachine.h>

USING_PTR(ComponentCPUState);

//! @brief CPUの行動を決めるステートマシン
//!
//! ■ 行動の流れ
//!   思考(THINK) → 状況と性格から次の行動を重み付き抽選 → 行動 → 思考 → ...
//!
//!   手ぶらの時   : 物を拾いに行く / キャラクターを掴みに行く / うろうろする / 狙われていたら避ける
//!   持っている時 : 狙う相手を選んで、投げられる距離まで近づき、相手の方を向いたら投げる
//!
//! ■ 自然に見せるための工夫
//!   - 性格(攻撃的/慎重/バランス)ごとに思考時間・避けやすさ・投げる距離・狙う相手の選び方が違う
//!   - 思考時間は毎回ランダム(全員が同時に動き出さない)
//!   - 行動中も周りを見て、拾う物を取られた・狙う相手が倒れた・狙われた時は行動を切り替える
//!   - 行動ごとに制限時間があり、詰まって動けなくなっても考え直す
class ComponentCPUState : public ComponentStateMachine
{
public:
    BP_COMPONENT_DECL(ComponentCPUState, u8"CPU状態制御コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

private:
    //---------------------------------------------------------------
    // 行動
    //---------------------------------------------------------------
    // CPUの行動定数
    enum class CPU_ACTION
    {
        ACTION_THINK = 0,
        // ものを掴みに行く動き(物・キャラクター)
        ACTION_GRAB,
        // ものを持っている相手を避ける動き
        ACTION_AVOID_ATTACKER,
        // ものを持っている場合、相手に投げに行く動き
        ACTION_ATTACK,
        // 低確率で回避のためにその場で真上にジャンプする動き
        ACTION_EVASION_JUMP,
        // やることが無い時に場所を変える動き
        ACTION_WANDER,
        ACTION_NUM
    };

    CPU_ACTION cur_action_  = CPU_ACTION::ACTION_THINK;
    CPU_ACTION prev_action_ = CPU_ACTION::ACTION_ATTACK;

    //---------------------------------------------------------------
    // 性格
    //---------------------------------------------------------------
    enum class CPU_PERSONALITY
    {
        AGGRESSIVE = 0,    //!< 攻撃的: すぐ動き、近い相手を遠くからでもどんどん狙う。あまり避けない
        CAUTIOUS,          //!< 慎重  : よく考え、よく避ける。弱っている相手を近くまで寄って確実に狙う
        BALANCED,          //!< バランス: その中間。操作プレイヤーを少し狙いやすい
        NUM
    };

    //! @brief 性格ごとのパラメータ
    struct PersonalityParam
    {
        const char* name;                     //!< GUI表示用の名前
        float       think_min;                //!< 思考時間の最小(秒)
        float       think_max;                //!< 思考時間の最大(秒)
        float       avoid_chance;             //!< 狙われた時に避ける確率(0～1)
        float       throw_range;              //!< この距離まで近づいたら投げる
        float       grab_character_weight;    //!< キャラクターを掴みに行く重み(物を拾う重みを1とした時)
        float       wander_weight;            //!< うろうろする重み(物を拾う重みを1とした時)
        float       target_near_weight;       //!< 狙う相手を選ぶ時、近さをどれだけ重視するか
        float       target_weak_weight;       //!< 狙う相手を選ぶ時、HPの少なさをどれだけ重視するか
        float       target_player_bonus;      //!< 狙う相手を選ぶ時、操作プレイヤーへのおまけ点
    };
    static const PersonalityParam PERSONALITY_PARAMS[static_cast<int>(CPU_PERSONALITY::NUM)];

    CPU_PERSONALITY personality_ = CPU_PERSONALITY::BALANCED;

    //! @brief 自分の性格のパラメータ
    const PersonalityParam& Param() const;

    //---------------------------------------------------------------
    // 行動の開始(ACTIONを追加する場合は開始関数とDecideActionの抽選を追加する)
    //---------------------------------------------------------------
    //! @brief 思考を始める
    //! @param duration 思考時間(秒)。負の値なら性格に合わせてランダムに決める
    void StartThink(float duration = -1.0f);
    void StartGrabAction(ObjectWeakPtr target);
    void StartAvoidAttacker(ObjectPtr attacker);
    void StartAttackAction(ObjectWeakPtr target);
    void StartEvadeJump();
    void StartWander();

    //! @brief 思考が終わった時に、状況と性格から次の行動を決める
    void DecideAction();

    //---------------------------------------------------------------
    // 行動中の更新(途中で行動を切り替えた場合はtrueを返す)
    //---------------------------------------------------------------
    void UpdateGrabAction();
    void UpdateAttackAction();
    void UpdateWalkEndAction();    //!< 歩き終わったら考え直すだけの行動(避ける・うろうろ)

    //! @brief 狙われていたら、確率で今の行動をやめて避ける
    //! @return 避ける行動に切り替えたらtrue
    bool TryInterruptByThreat();

    //! @brief 回避ジャンプを発動するか(低確率・クールダウンあり)
    bool IsEvadeJumpTrigger();

    //---------------------------------------------------------------
    // 周りの状況を調べる
    //---------------------------------------------------------------
    //! @brief 物を持ち上げ終わって手に持っているか(持ち上げ中は含まない)
    bool IsHolding() const;

    //! @brief 自分を狙っている(物を持ってこちらを向いている)一番近い相手
    ObjectPtr GetThreat();

    //! @brief 投げる相手を選ぶ(近さ・HP・操作プレイヤーかどうか・ランダムで点数をつける)
    ObjectWeakPtr ChooseAttackTarget();

    //! @brief 掴みに行く対象を選ぶ
    //! @param character true: キャラクターから選ぶ / false: 岩・丸太・スライムなどの物から選ぶ
    ObjectWeakPtr ChooseGrabTarget(bool character);

    //! @brief 掴みに行く対象がまだ掴める状態か
    bool IsGrabTargetValid(const ObjectPtr& target) const;

    //! @brief 自分の正面が対象の方向を向いているか
    //! @param max_angle_deg この角度以内なら向いているとみなす(度)
    bool IsFacing(const ObjectPtr& target, float max_angle_deg) const;

    //! @brief 視線判定で捉えた掴み対象に手が届いているか(体が触れていて正面にある物だけが候補になる)
    bool IsEyeSightTargetInReach() const;

    //! @brief ダウンから復帰した(ComponentStateMachine から呼ばれる)
    void OnRecover() override;

    //! @brief 2点間のXZ平面上の距離
    static float DistanceXZ(const float3& a, const float3& b);

    //! @brief a～bのランダムな値
    static float RandomRange(float a, float b);

    //---------------------------------------------------------------
    // パラメータ・状態
    //---------------------------------------------------------------
    ObjectWeakPtr target_want_grab_;    //!< 掴みに行っている対象
    ObjectWeakPtr attack_target_;       //!< 投げようとしている相手

    float action_timer_       = 0.0f;    //!< 今の行動を始めてからの時間(秒)
    float threat_check_timer_ = 0.0f;    //!< 狙われているかを次に調べるまでの時間(秒)
    float avoid_cooldown_     = 0.0f;    //!< 避けた後、次に避けられるまでの時間(秒)

    const float grab_timeout_      = 6.0f;     //!< 掴みに行く行動の制限時間(秒)
    const float attack_timeout_    = 7.0f;     //!< 投げに行く行動の制限時間(秒)
    const float walk_timeout_      = 4.0f;     //!< 避ける・うろうろする行動の制限時間(秒)
    const float threat_distance_   = 40.0f;    //!< この距離以内で物を持ってこちらを向いている相手を脅威とみなす
    const float threat_angle_      = 35.0f;    //!< 相手の正面からこの角度以内にいたら狙われているとみなす(度)
    const float avoid_cooldown_time_ = 2.5f;   //!< 避けた後のクールダウン(秒)
    const float grab_reach_        = 12.0f;    //!< 手が届くとみなす距離
    const float arena_radius_      = 95.0f;    //!< うろうろ・避ける時にこの半径の外へ出ないようにする(ステージ半径は約120)
    const float grab_search_range_ = 150.0f;   //!< この距離以内の物を拾いに行く
    const float aim_angle_         = 25.0f;    //!< 投げる前に相手の方向をどれだけ正確に向くか(度)

    // 回避ジャンプのパラメータ
    const float evasion_jump_threat_distance_ = 10.0f;    // この距離以内に攻撃者がいると脅威とみなす
    const int   evasion_jump_chance_percent_  = 30;       // 発動確率(%)
    const float evasion_jump_cooldown_        = 4.0f;     // 発動後のクールダウン(秒)
    float       evasion_jump_cooldown_timer_  = 0.0f;

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentCPUState, 1);
