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

    //! @brief 物理シミュレーション後処理
    //! 移動量に基づいて視覚的な転がり回転を適用する
    void PostPhysics() override;

private:
    bool is_bounced_this_frame_ = false;    //同一フレーム内でのバウンド重複防止フラグ
    int  se_handle_             = -1;       //SEのメモリハンドル
    float throw_cool_down_      = 0.0f;     //SE連続再生防止クールダウンタイマー

    float3 prev_pos_{0, 0, 0};    // 物理演算前の位置（転がり回転計算用）
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Log);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Log);
