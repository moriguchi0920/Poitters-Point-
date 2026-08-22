#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/State/ComponentState.h>
#include <Game/Component/State/ComponentStateWalkBase.h>

USING_PTR(ComponentStateControllerWalk);

class ComponentStateControllerWalk : public ComponentStateWalkBase
{
public:
    BP_COMPONENT_DECL(ComponentStateControllerWalk, u8"停止・歩きコンポーネント");

    void Init() override;

    void Update() override;

    ComponentStateControllerWalkPtr SetKeys(int up, int down, int left, int right);

    void GUI() override;

private:
    int key_up_    = KEY_INPUT_W;
    int key_down_  = KEY_INPUT_S;
    int key_left_  = KEY_INPUT_A;
    int key_right_ = KEY_INPUT_D;

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

            CEREAL_NVP(key_up_),
            CEREAL_NVP(key_down_),
            CEREAL_NVP(key_left_),
            CEREAL_NVP(key_right_),

            CEREAL_NVP(front_rot_));

        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
    }
};

CEREAL_CLASS_VERSION(ComponentStateControllerWalk, 1);
