#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentStatePlayerJump.h
//! @brief  ジャンプステート(プレイヤー/CPU共通)
//! @details 踏ん張り → 上向きの初速を与えて物理(重力)で放物線 → 着地(接地判定) → 硬直 の流れ。
//---------------------------------------------------------------------------
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

    ComponentStatePlayerJumpPtr SetJumpSpeed(float speed);
    ComponentStatePlayerJumpPtr SetAirMoveSpeed(float speed);
    ComponentStatePlayerJumpPtr SetRotateSpeed(float speed);
    // ジャンプ
    ComponentStatePlayerJumpPtr SetKeys(int up, int down, int left, int right, int jump = KEY_INPUT_SPACE);

    float GetJumpSpeed() const { return jump_speed_; }
    float GetAirMoveSpeed() const { return air_move_speed_; }

    inline void  SetFrontRotate(float rotate) { front_rot_ = rotate; }
    inline float GetFrontRotate() const { return front_rot_; }

    void GUI() override;

    void SetIsHolding(bool hold);
    bool IsLanding() const { return is_landing_; }

    //! CPUのジャンプとして扱う設定(CPUは入力キーを読まず直上ジャンプする)
    void SetIsCPU(bool cpu);
    bool GetIsCPU() const { return is_cpu_; }

private:
    //! @brief 着地硬直が終わった後の処理(歩きへ戻る / CPUは自分を消すだけ)
    void FinishJump();

    // ジャンプパラメータ
    float jump_speed_     = 120.0f;    // 上向き初速(単位/秒)。重力 約294 で高さ約25(キャラの身長くらい)
    float air_move_speed_ = 0.4f;      // 空中の水平移動(1/60秒あたり)
    float rot_speed_      = 20.0f;     // 回転速度

    // 地上での踏ん張りタイマー
    float startup_timer_ = 0.25f;

    float3 jump_direction_{0, 0, 0};
    bool   is_jumped_     = false;    // 踏み切ったか
    float  air_time_      = 0.0f;     // 踏み切ってからの時間
    bool   is_landing_    = false;
    float  landing_Timer_ = 0.0f;

    // キー設定（デフォルトでスペースキーを設定）
    int key_up_    = KEY_INPUT_W;
    int key_down_  = KEY_INPUT_S;
    int key_left_  = KEY_INPUT_A;
    int key_right_ = KEY_INPUT_D;
    int key_jump_  = KEY_INPUT_SPACE;    // ジャンプキー

    float front_rot_  = 0.0f;    // 前方ベクトルの回転角度(0-360度)
    bool  is_holding_ = false;
    bool  is_cpu_     = false;    // CPU用ジャンプか(セーブ対象ではない)

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ/ロード
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(CEREAL_NVP(jump_speed_),
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
