#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

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
    float lift_time_ = 1.0f;     //!< 持ち上げにかかる残り時間
    bool  finished_  = false;    //!< 持ち上げモーションが終わったか(未初期化だと即完了扱いになる)
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
