#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Bullet.h
//! @brief  PoittersPoint_Bullet
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Bullet);
class Bullet : public Object
{
public:
    BP_OBJECT_DECL(Bullet, "PoittersPoint::Bullet");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief 更新
    //! キーを確認して弾を撃たせるために必要
    void Update() override;

    //! @brief GUI
    //! GUIでの弾の速度調整のために必要
    void GUI() override;

    //! @brief 方向の設定
    void SetDirection(float3 dir);

    //! @brief 自身の方向で初期化
    void ResetDirection();

    //! @brief 当たり判定のコールバック
    //! 当たり判定が行われたときに自動で呼び出される関数
    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

private:
    // 弾の移動方向
    float3 direction_ = {0, 1, 0};

    //! 生存フレーム数
    float life_ = 5.0f;
    //! 弾のスピード
    float speed_ = 2.0f;
    // コインの回転角度
    float spin_angle_ = 0.0f;

    // 弾の存在時間(秒)[全部の弾の共通]
    static float max_life_;
    // 弾の速度[全部の弾の共通]
    static float max_speed_;

    // Z軸の回転速度
    static float max_rotateX_speed_;
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Bullet);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Bullet);
