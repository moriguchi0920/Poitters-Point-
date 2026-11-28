#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentGrabbable.h
//! @brief  掴み可能コンポーネント(投げ物・キャラクター共通)
//! @details 物理は全て ComponentPhysics に任せ、このクラスは
//!          「誰が持っているか / 投げられて飛んでいるか」の状態と、
//!          投げられた物が当たった時のダメージ・ノックバックだけを担当する。
//!
//!   状態遷移:
//!     Free ──Reserve()──▶ Reserved(持ち上げモーション中) ──Hold()──▶ Held(手に持っている)
//!       ▲                    │CancelReserve()                       │Throw() / Drop()
//!       └────────────────────┴──────── 止まったら ◀── Thrown(飛んでいる/転がっている)
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentPhysics.h>

USING_PTR(ComponentGrabbable);

class ComponentGrabbable : public Component
{
public:
    BP_COMPONENT_DECL(ComponentGrabbable, u8"掴み可能コンポーネント");

    //! 掴み状態
    enum class State
    {
        Free,        //!< 置かれている(止まっていれば掴める)
        Reserved,    //!< 誰かが持ち上げ始めている
        Held,        //!< 誰かが手に持っている
        Thrown,      //!< 投げられて(または落とされて)動いている
    };

    void Init() override;
    void Update() override;
    void PrePhysics() override;
    void GUI() override;

    using Component::OnHitComponent;
    //! @brief 物理の接触通知(投げられて飛んでいる間のヒット判定)
    void OnHitComponent(const HitInfoPhysics& hit_info) override;

    //------------------------------------------------------------------
    //! @name パラメータ
    //------------------------------------------------------------------
    //@{
    void         SetLiftTime(float time) { lift_time_ = time; }
    const float& GetLiftTime() const { return lift_time_; }

    void  SetDamage(float damage) { damage_ = damage; }
    float GetDamage() const { return damage_; }

    //! @brief 当たった相手を吹き飛ばす強さ(水平速度 単位/秒)
    void  SetKnockbackPower(float power) { knockback_power_ = power; }
    float GetKnockbackPower() const { return knockback_power_; }

    //! @brief 投げる速さの倍率(重い物は小さく)
    void  SetThrowSpeedScale(float scale) { throw_speed_scale_ = scale; }
    float GetThrowSpeedScale() const { return throw_speed_scale_; }

    //! @brief 投げた時に進行方向へ転がる回転を与える量(0:なし 1:地面を転がる回転と同じ)
    //! @param radius 転がる半径(物理形状の半径)
    void SetRollSpin(float spin, float radius)
    {
        roll_spin_   = spin;
        roll_radius_ = radius;
    }

    //! @brief 投げる時に円柱の軸(ボディY軸)を進行方向と直交させる(丸太が転がる向きにする)
    void SetAlignRollAxisOnThrow(bool align) { align_roll_axis_ = align; }
    //@}

    //------------------------------------------------------------------
    //! @name 状態の取得
    //------------------------------------------------------------------
    //@{
    State GetState() const { return state_; }
    bool  IsHeld() const { return state_ == State::Held; }
    bool  IsThrown() const { return state_ == State::Thrown; }
    //! @brief 互換用：手に持たれているか
    bool GetIsGrabbed() const { return IsHeld(); }

    //! @brief 今このオブジェクトを掴めるか
    //! @details 置かれていて(Free)、止まっていて、誰も持ち上げ始めていないこと。
    //!          キャラクターの場合はモーション中・物を持っている間・空中も不可
    bool CanBeGrabbed() const;

    //! @brief 動いているか(速度がしきい値以上)
    bool IsMoving() const;

    //! @brief 投げた本人か
    bool IsThrower(const ObjectPtr& target) const;

    //! @brief 地面(または置き物)に支えられているか(最近触れているか)
    //! @details 物理ステップが無いフレームでもちらつかないよう、少しの間は触れているとみなす。
    //!          置かれて止まった物(Kinematic)は地面との接触通知が来ないため、常に支えられているとみなす
    bool IsSupported() const { return is_resting_ || time_since_support_ <= 0.15f; }
    //@}

    //------------------------------------------------------------------
    //! @name 操作 (ComponentStateMachine から呼ぶ)
    //------------------------------------------------------------------
    //@{
    //! @brief 持ち上げを予約する(他の人は掴めなくなる)
    //! @retval false 掴めない状態だった
    bool Reserve(const ObjectPtr& lifter);

    //! @brief 持ち上げの予約を取り消す(持ち上げ中に攻撃された等)
    void CancelReserve();

    //! @brief 手に持つ(予約した本人のみ)
    void Hold(const ObjectPtr& holder, std::string_view bone = "mixamorig:RightHand");

    //! @brief 投げる
    //! @param velocity 初速(単位/秒)。ThrowSpeedScale が掛かる
    void Throw(const ObjectPtr& thrower, const float3& velocity);

    //! @brief その場に落とす(ダメージなし)
    void Drop();
    //@}

    //! @brief 投げられて飛んでいる間に何かへ接触した時の処理 (スライムの爆発など)
    std::function<void(Object* hit_object)> OnImpactFunc;

private:
    std::shared_ptr<physics::RigidBody> GetBody() const;

    //! @brief 手から離す共通処理(AttachModel除去・Dynamic化・すり抜け解除)
    void ReleaseFromHand();

    //! @brief 置かれて止まった物を「動かない物」(Kinematic)にする / 戻す
    //! @details 止まっている物はキャラクターが体で押しても動かないようにする。
    //!          投げられた物が当たった時は Dynamic に戻して弾き飛ばされる
    void SetResting(bool resting);

    //! @brief 置き物として扱うか(キャラクター以外)
    bool IsProp() const;

    //! @brief 手のボーン行列から、物理ボディを置くべき行列を求める(見た目と当たり判定を一致させる)
    matrix CalcHeldBodyMatrix() const;

    //! @brief 投げ物が target に当たった時のダメージ・ノックバック
    void HitTarget(Object* target);

    //! @brief 既にこの投げで当てた相手か
    bool IsAlreadyHit(const Object* target) const;

    State         state_ = State::Free;
    ObjectWeakPtr holder_;     //!< 持ち上げ中/持っている人
    ObjectWeakPtr thrower_;    //!< 最後に投げた人(この人には当たらない)

    std::vector<ObjectWeakPtr> hit_targets_;    //!< この投げで既に当てた相手(連続ヒット防止)

    bool   can_hit_       = false;    //!< 投げ中に当たり判定を行うか(落とした時は行わない)
    bool   is_resting_    = false;    //!< 置かれて止まり、Kinematic になっているか
    float  time_since_support_ = 999.0f;    //!< 地面(または置き物)に最後に触れてからの時間
    float  rest_time_     = 0.0f;     //!< 止まっている時間
    float  thrown_time_   = 0.0f;     //!< 投げてからの時間
    float3 last_velocity_ = {0.0f, 0.0f, 0.0f};    //!< 物理ステップ前の速度(当たった瞬間の勢い)
    u16    saved_overlap_ = 0;                     //!< 持つ前のすり抜け設定

    //--- パラメータ ---
    float lift_time_            = 1.0f;      //!< 持ち上げにかかる時間
    float damage_               = 1.0f;      //!< 当たった時のダメージ
    float knockback_power_      = 120.0f;    //!< 当たった相手を吹き飛ばす水平速度
    float knockback_up_         = 70.0f;     //!< 当たった相手を浮かせる上向き速度
    float throw_speed_scale_    = 1.0f;      //!< 投げる速さの倍率
    float roll_spin_            = 0.0f;      //!< 投げた時の転がり回転の量
    float roll_radius_          = 1.0f;      //!< 転がる半径
    bool  align_roll_axis_      = false;     //!< 投げる時に円柱軸を進行方向と直交させる
    float move_speed_threshold_ = 20.0f;     //!< 動いているとみなす速さ(単位/秒)
    float rest_judge_time_      = 0.15f;     //!< この時間止まっていたら置かれた(Free)とみなす

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
    //@}
};

CEREAL_CLASS_VERSION(ComponentGrabbable, 1);
