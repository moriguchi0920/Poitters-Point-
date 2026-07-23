#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

USING_PTR(ComponentStateIdleWalk);

class ComponentStateIdleWalk : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateIdleWalk, u8"停止・歩きコンポーネント");

    void Init() override;

    void Update() override;

    ComponentStateIdleWalkPtr SetMoveSpeed(const float speed);

    ComponentStateIdleWalkPtr SetRotateSpeed(const float speed);

    ComponentStateIdleWalkPtr SetKeys(int up, int down, int left, int right);

    const float GetMoveSpeed() const;
    const float GetRotateSpeed() const;

    inline const void  SetFrontRotate(float rotate) { front_rot_ = rotate; }
    inline const float GetFrontRotate() const { return front_rot_; }

    void GUI() override;

    void SetIsHolding(bool hold);

private:
    float move_speed_ = 0.3f;
    float rot_speed_  = 20.0f;

    int key_up_    = KEY_INPUT_W;
    int key_down_  = KEY_INPUT_S;
    int key_left_  = KEY_INPUT_A;
    int key_right_ = KEY_INPUT_D;

    float front_rot_ = 0.0f;    //!<前方ベクトルの回転角度(0-360度)

    bool is_holding_;

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

CEREAL_CLASS_VERSION(ComponentStateIdleWalk, 1);
