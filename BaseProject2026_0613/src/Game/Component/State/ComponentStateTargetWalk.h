#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/State/ComponentState.h>
#include <Game/Component/State/ComponentStateWalkBase.h>

USING_PTR(ComponentStateTargetWalk);

class ComponentStateTargetWalk : public ComponentStateWalkBase
{
public:
    BP_COMPONENT_DECL(ComponentStateTargetWalk, u8"CPU用指定地点への歩きコンポーネント");

    void Init() override;

    void Update() override;

    ComponentStateTargetWalkPtr SetTargetPos(const float3& position);
    void                        SetTargetPtr(const ObjectPtr ptr);
    void                        ResetTargetPtr();
    const float3&               GetTargetPos();
    bool                        GetArrival();

    void GUI() override;
    bool GetStopped();

private:
    float3    target_pos_;
    ObjectPtr target_ptr_;

    float       stopping_time_;
    const float stop_judge_time_ = 0.5f;
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
