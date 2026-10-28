#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Log.h
//! @brief  PoittersPoint_Log
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Log);
class Log : public Object
{
public:
    BP_OBJECT_DECL(Log, "PoittersPoint::Log");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief 更新
    //! キーを確認して弾を撃たせるために必要
    void Update() override;

    //! @brief GUI
    //! GUIでの弾の速度調整のために必要
    void GUI() override;

    //! @brief 当たり判定のコールバック
    //! 当たり判定が行われたときに自動で呼び出される関数
    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

    //! @brief 物理シミュレーション前処理
    //! 投擲後の移動速度を適用する
    void PrePhysics() override;

    //! @brief 物理シミュレーション後処理
    //! 移動量に基づいて視覚的な転がり回転を適用する
    void PostPhysics() override;

private:
    bool  is_bounced_this_frame_ = false;    //同一フレーム内でのバウンド重複防止フラグ
    int   se_handle_             = -1;       //SEのメモリハンドル
    float throw_cool_down_       = 0.0f;     //SE連続再生防止クールダウンタイマー

    float3 prev_pos_{0, 0, 0};           // 物理演算前の位置（転がり回転計算用）
    float3 velocity_{0, 0, 0};           // 転がり用の水平速度
    float3 throw_velocity_{0, 0, 0};     // 投擲速度（空中飛行用）
    bool   was_grabbed_prev_ = false;    // 投擲検出用：前フレームの掴まれ状態
    bool   just_thrown_      = false;    // 投擲直後のフレーム：速度適用をスキップ
    bool   is_on_ground_     = false;    // 接地状態（持続）
    bool   was_on_ground_    = false;    // 前フレームの接地状態

    //--- 挙動調整用パラメータ ---
    float k_air_drag_     = 0.999f;    //!< 空気抵抗（空中時の速度減衰率）
    float k_friction_     = 0.96f;     //!< 転がり摩擦（接地時の速度減衰率）
    float k_landing_loss_ = 0.7f;      //!< 着地時に失う水平速度の割合
    float k_throw_up_     = 3.0f;      //!< 投擲時の上方向衝動
    float k_roll_radius_  = 3.0f;      //!< 転がり半径（コライダー半径と一致）
    float k_throw_power_  = 0.2f;      //!< 投擲速度のスケーリング
    float k_hop_cancel_   = 1.53f;     //!< 投擲フレームの水平相殺率（develop側のAddTranslate分を含む）
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Log);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Log);
