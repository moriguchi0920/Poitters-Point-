#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/State/ComponentState.h>

USING_PTR(ComponentStateTargetWalk);

class ComponentStateTargetWalk : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateTargetWalk, u8"CPU用指定地点への歩きコンポーネント");

    void Init() override;

    void Update() override;

    ComponentStateTargetWalkPtr SetMoveSpeed(const float speed);

    ComponentStateTargetWalkPtr SetRotateSpeed(const float speed);

    ComponentStateTargetWalkPtr SetKeys(int up, int down, int left, int right);

    const float GetMoveSpeed() const;
    const float GetRotateSpeed() const;

    inline const void  SetFrontRotate(float rotate) { front_rot_ = rotate; }
    inline const float GetFrontRotate() const { return front_rot_; }

    void          SetTargetPos(const float3& position);
    const float3& GetTargetPos();

    void GUI() override;

    void SetIsHolding(bool hold);

private:
    float move_speed_ = 1.0f;
    float rot_speed_  = 5.0f;

    int key_up_    = KEY_INPUT_W;
    int key_down_  = KEY_INPUT_S;
    int key_left_  = KEY_INPUT_A;
    int key_right_ = KEY_INPUT_D;

    float front_rot_ = 0.0f;    //!<前方ベクトルの回転角度(0-360度)

    float3 target_pos_;

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

CEREAL_CLASS_VERSION(ComponentStateTargetWalk, 1);
