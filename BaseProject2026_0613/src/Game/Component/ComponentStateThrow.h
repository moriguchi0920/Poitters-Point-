#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentStateThrow.h
//! @brief  投げステート
//! @details 投げモーションを再生し、手を離すタイミングで ComponentStateMachine::ReleaseThrow を呼ぶ。
//!          空中投げでも落下は物理(重力)に任せるため、ここでは高さを扱わない。
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

USING_PTR(ComponentStateThrow);

class ComponentStateThrow : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateThrow, u8"投擲攻撃コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    bool GetIsFinished();

    //! @brief 空中投げか(上半身だけの速い投げモーションを使う)
    void SetInAir(bool in_air) { is_in_air_ = in_air; }

private:
    //! @brief 投げる初速を計算する(正面 + 少し上向き)
    float3 CalcThrowVelocity() const;

    //! @brief 投げモーションが終わったか
    bool IsMotionEnd() const;

    bool is_in_air_   = false;
    bool is_released_ = false;    //!< 手離しが完了したか
    bool is_started_  = false;    //!< モーション再生を開始したか

    float throw_speed_    = 160.0f;    //!< 正面方向の初速(単位/秒)
    float throw_up_speed_ = 60.0f;     //!< 上向きの初速(単位/秒)

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
    //@}
};

CEREAL_CLASS_VERSION(ComponentStateThrow, 1);
