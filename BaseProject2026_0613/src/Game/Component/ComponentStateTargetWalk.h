#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>
#include <Game/Component/ComponentStateWalkBase.h>

USING_PTR(ComponentStateTargetWalk);

class ComponentStateTargetWalk : public ComponentStateWalkBase
{
public:
    BP_COMPONENT_DECL(ComponentStateTargetWalk, u8"CPU用指定地点への歩きコンポーネント");

    void Init() override;

    void Update() override;

    ComponentStateTargetWalkPtr SetTargetPos(const float3& position);
    void                        SetTargetPtr(const ObjectWeakPtr ptr);
    void                        ResetTargetPtr();
    const float3&               GetTargetPos();
    //! @brief 「近くに着いた(GetNearby)」とみなす水平距離を設定する(拾う時は近く、投げる時は射程)
    void SetNearbyDistance(float distance) { nearby_distance_ = distance; }
    bool                        GetArrival() override;

    void GUI() override;
    bool GetStopped() override;
    bool GetNearby() override;

private:
    float3        target_pos_ = {0.0f, 0.0f, 0.0f};
    float3        prev_pos_   = {0.0f, 0.0f, 0.0f};    //!< 前フレームの位置(引っかかり判定用)
    ObjectWeakPtr target_ptr_;

    float       stopping_time_   = 0.0f;
    float       nearby_distance_ = 10.0f;    //!< 近くに着いたとみなす水平距離
    const float stop_judge_time_ = 1.0f;
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(CEREAL_NVP(move_speed_),
            CEREAL_NVP(rot_speed_),

            CEREAL_NVP(front_rot_));

        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
    }
};

CEREAL_CLASS_VERSION(ComponentStateTargetWalk, 1);
