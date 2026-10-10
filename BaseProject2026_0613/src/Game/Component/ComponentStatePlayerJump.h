#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

USING_PTR(ComponentStatePlayerJump);

class ComponentStatePlayerJump : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStatePlayerJump, u8"プレイヤー・ジャンプコンポーネント");

    void Init() override;

    void Update() override;

    ComponentStatePlayerJumpPtr SetJumpPower(float power);
    ComponentStatePlayerJumpPtr SetGravity(float gravity);
    ComponentStatePlayerJumpPtr SetAirMoveSpeed(float speed);
    ComponentStatePlayerJumpPtr SetRotateSpeed(float speed);
    // ジャンプ
    ComponentStatePlayerJumpPtr SetKeys(int up, int down, int left, int right, int jump = KEY_INPUT_SPACE);

    float GetJumpPower() const;
    float GetGravity() const;
    float GetAirMoveSpeed() const;

    inline void  SetFrontRotate(float rotate) { front_rot_ = rotate; }
    inline float GetFrontRotate() const { return front_rot_; }

    void GUI() override;

    void SetIsHolding(bool hold);
    bool IsLanding() const { return is_landing_; }

private:
    // ジャンプパラメータ
    float jump_power_     = 2.0f;     // ジャンプの高さを出す
    float gravity_        = 0.05f;    // 重力をかけることで僅かな滞空時間を作る
    float air_move_speed_ = 0.4f;
    float rot_speed_      = 20.0f;    // 回転速度

    // 地上での踏ん張りタイマー
    float startup_timer_ = 0.25f;

    float3 jump_direction_{0, 0, 0};
    float  current_jump_velocity_ = 0.0f;
    float  ground_y_              = 0.0f;
    bool   is_landing_            = false;
    float  landing_Timer_         = 0.0f;

    // キー設定（デフォルトでスペースキーを設定）
    int key_up_    = KEY_INPUT_W;
    int key_down_  = KEY_INPUT_S;
    int key_left_  = KEY_INPUT_A;
    int key_right_ = KEY_INPUT_D;
    int key_jump_  = KEY_INPUT_SPACE;    // ジャンプキー

    float front_rot_  = 0.0f;    // 前方ベクトルの回転角度(0-360度)
    bool  is_holding_ = false;

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ/ロード
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(CEREAL_NVP(jump_power_),
            CEREAL_NVP(gravity_),
            CEREAL_NVP(air_move_speed_),
            CEREAL_NVP(rot_speed_),
            CEREAL_NVP(startup_timer_),

            CEREAL_NVP(key_up_),
            CEREAL_NVP(key_down_),
            CEREAL_NVP(key_left_),
            CEREAL_NVP(key_right_),
            CEREAL_NVP(key_jump_),

            CEREAL_NVP(front_rot_));

        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
    }
};

CEREAL_CLASS_VERSION(ComponentStatePlayerJump, 1);
