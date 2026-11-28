#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Log.h
//! @brief  PoittersPoint_Log (投げ物：丸太)
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

    //! @brief 更新(SEのクールダウン)
    void Update() override;

    //! @brief 物理シミュレーション前処理(着地の勢い判定用に速度を記録)
    void PrePhysics() override;

    //! @brief 終了(SEの解放)
    void Exit() override;

    //! @brief GUI
    void GUI() override;

    //! @brief 物理の当たり判定のコールバック(着地SE)
    void OnHitPhysics(const ComponentPhysics::HitInfo& hit_info) override;

private:
    int    se_handle_            = -1;      //!< SEのメモリハンドル
    float  se_cool_down_         = 0.0f;    //!< SE連続再生防止クールダウンタイマー
    float  se_min_speed_         = 40.0f;   //!< この速さ以上でぶつかったらSEを鳴らす
    float3 velocity_before_step_ = {0.0f, 0.0f, 0.0f};

    //--- 物理形状(モデルの大きさから自動計算。取得できない時の既定値) ---
    float radius_      = 6.0f;      //!< 円柱の半径
    float half_length_ = 23.2f;     //!< 円柱の長さの半分
    float density_     = 600.0f;    //!< 密度(木なので岩より軽い)
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Log);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Log);
