#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Ground.h
//! @brief  PoittersPoint_Ground
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Ground);
class Ground : public Object
{
public:
    BP_OBJECT_DECL(Ground, "PoittersPoint::Ground");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;
    
    //// ! @brief 当たり判定のコールバック
    ////! 当たり判定が行われたときに自動で呼び出される関数
    // void OnHit(const ComponentCollision::HitInfo& hit_info) override;


    //! @brief 描画（独自描画やシェーダー処理を行う場合）
    void Draw() override;

//
//    // ステージ（円）の半径を取得・設定するアクセサ
//    float GetRadius() const { return m_radius; }
//    void  SetRadius(float radius) { m_radius = radius; }
//
private:
    // 円の範囲（半径）を管理する変数
    float m_radius = 120.0f;
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Ground);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Ground);
