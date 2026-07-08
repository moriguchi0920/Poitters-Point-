#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Enemy.h
//! @brief  PoittersPoint_Enemy
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Enemy);
class Enemy : public Object
{
public:
    BP_OBJECT_DECL(Enemy, "PoittersPoint::Enemy");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief
    void Update();

    //! @brief 当たり判定のコールバック
    //! 当たり判定が行われたときに自動で呼び出される関数
    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

    bool is_dead = false;

    bool is_down = false;

    bool is_running = false;    // 現在走っているかどうかのフラグ
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Enemy);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Enemy);
