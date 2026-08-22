#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/State/ComponentState.h>

USING_PTR(ComponentStateGrab);

class ComponentStateGrab : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateGrab, u8"つかみ状態コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    void SetLiftTime(float time);

    bool GetIsFinished();

private:
    float lift_time_;

    bool finished_;
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentStateGrab, 1);
