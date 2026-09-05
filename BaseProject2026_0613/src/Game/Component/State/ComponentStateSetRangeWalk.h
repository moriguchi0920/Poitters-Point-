#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/State/ComponentState.h>
#include <Game/Component/State/ComponentStateWalkBase.h>

USING_PTR(ComponentStateSetRangeWalk);

class ComponentStateSetRangeWalk : public ComponentStateWalkBase
{
public:
    BP_COMPONENT_DECL(ComponentStateSetRangeWalk, u8"距離・方向を指定しての歩きコンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    void SetWalkDirection(const float3& direction);
    void SetWalkDistance(const float& distance);

    bool GetArrival();
    bool GetStopped();

protected:
    // 移動方向
    float3 walk_direction_;
    // 移動距離
    float walk_distance_;

    float       stopping_time_;
    const float stop_judge_time_ = 0.5f;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(CEREAL_NVP(move_speed_), CEREAL_NVP(rot_speed_), CEREAL_NVP(front_rot_));

        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
    }
};

CEREAL_CLASS_VERSION(ComponentStateSetRangeWalk, 1);
