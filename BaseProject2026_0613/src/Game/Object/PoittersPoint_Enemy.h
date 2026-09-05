#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Enemy.h
//! @brief  PoittersPoint_Enemy
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Object.h>
#include <System/Component/ComponentCollision.h>

namespace PoittersPoint {

class Enemy;
USING_PTR(Enemy);

class Enemy : public Object
{
public:
    BP_OBJECT_DECL(Enemy, u8"エネミー");

    //! @brief 初期化
    bool Init() override;

    //! @brief 更新
    void Update() override;

    //! @brief 当たり判定のコールバック
    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

    bool is_dead    = false;    //!< 死亡済みかどうか
    bool is_down    = false;    //!< ダウン状態かどうか
    bool is_running = false;    //!< 現在走っているかどうかのフラグ
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Enemy);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Enemy);
