#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentStateMachine.h
//! @brief  キャラクターのステートマシン基底 (プレイヤー入力/CPU 共通)
//! @details 掴む・投げる・ダウンの流れはここで共通化し、
//!          派生クラス(ComponentPlayerState / ComponentCPUState)は「いつ行動するか」だけを決める。
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

USING_PTR(ComponentStateMachine);

class ComponentStateThrow;

class ComponentStateMachine : public Component
{
public:
    BP_COMPONENT_DECL(ComponentStateMachine, u8"ステートマシンコンポーネント");

    void Init() override;

    //! @brief ダウンからの復帰を監視する (派生クラスは最初に __super::Update() を呼ぶ)
    void Update() override;

    void GUI() override;

    //! @brief 現在のステートを T に変更する(ステートが無ければ追加する)
    template <class T>
    T* ChangeState()
    {
        static_assert(std::derived_from<T, ComponentState>, "ステートコンポーネント以外は指定できません");
        if(auto state = GetOwner()->GetComponent<ComponentState>()) {
            return state->ChangeState<T>();
        }
        return GetOwner()->AddComponent<T>().get();
    }

    const std::string GetStateName() const;

    //------------------------------------------------------------------
    //! @name 掴む・投げる
    //------------------------------------------------------------------
    //@{
    //! @brief 視線で見つけた掴み候補を登録する
    bool GrabbableHit(ObjectPtr target);
    //! @brief 掴み候補を消す(視線内に何もない)
    void ClearGrabCandidate() { grab_candidate_.reset(); }
    //! @brief 掴み候補がいるか
    bool IsGrabTargetExist() const { return !grab_candidate_.expired(); }

    //! @brief 掴み候補の持ち上げを開始する
    bool StartGrab();
    //! @brief 持ち上げモーションが終わったら手に持つ
    //! @retval true 持ち上げステートが終了した(持てたかは IsHolding() で確認)
    bool FinishGrab();

    //! @brief 持っている物を投げるステートへ移行する
    //! @param in_air 空中投げか
    //! @return 投げステート(投げられない時は nullptr)
    ComponentStateThrow* StartThrow(bool in_air);

    //! @brief 投げステートから呼ばれる：手を離して投げる
    void ReleaseThrow(const float3& velocity);

    //! @brief 持ち上げ中/保持中の物を投げずにその場で手放す(脱落時などに使う)
    void DropGrabbingObject() { DropHolding(); }

    //! @brief 指定のオブジェクトを持ち上げ中/保持中か
    bool IsGrabbingObject(const ObjectPtr& target) const { return target && grabbing_object_ptr_.lock() == target; }

    //! @brief 何かを持っている(持ち上げ中を含む)
    bool IsHolding() const { return !grabbing_object_ptr_.expired(); }
    //! @brief 互換用
    bool GetGrabbing() const { return IsHolding(); }
    //! @brief 投げられる状態か(手に持ち終わっている)
    bool CanThrow() const { return can_throw_ && IsHolding(); }

    //! @brief モーション中(持ち上げ/投げ/ジャンプ/ダウン)か、何かを持っている
    bool IsBusy() const;
    //@}

    //------------------------------------------------------------------
    //! @name 被弾・被掴み (ComponentGrabbable から呼ばれる)
    //------------------------------------------------------------------
    //@{
    //! @brief 投げ物に当たった：持ち物を落として吹き飛び、ダウンする
    void KnockDown(const float3& velocity);
    //! @brief 自分が誰かに持ち上げられ始めた
    void OnGrabbed();
    //! @brief 掴まれていた状態から解放された
    //! @param thrown true:投げられた/落とされた(着地後ダウン) false:持ち上げが取り消された(すぐ復帰)
    void OnReleased(bool thrown);
    //! @brief ダウン中(掴まれている間を含む)か
    bool IsDown() const;
    //! @brief HPが0になって脱落したか
    bool IsEliminated() const;
    //@}

protected:
    //! @brief ダウンから復帰した時に呼ばれる(派生クラスで通常のステートへ戻す)
    virtual void OnRecover() {}

    //! @brief 持っている物を落とす(持ち上げ中なら予約を取り消す)
    void DropHolding();

    //! @brief 持ち上げにかかる時間の倍率(キャラクター性能)
    float GetLiftTimeRatio() const;

    ObjectWeakPtr grab_candidate_;         //!< 視線内の掴み候補
    ObjectWeakPtr grabbing_object_ptr_;    //!< 持ち上げ中/持っている物
    bool          can_throw_ = false;      //!< 手に持ち終わって投げられる

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
    //@}
};

CEREAL_CLASS_VERSION(ComponentStateMachine, 1);
