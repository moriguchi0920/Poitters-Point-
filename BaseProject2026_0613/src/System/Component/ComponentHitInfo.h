//---------------------------------------------------------------------------
//! @file   ComponentHitInfo.h
//! @brief  コンポーネント当たり情報 (Componentではない)
//---------------------------------------------------------------------------
#pragma once
#include "ComponentCollision.h"
#include "ComponentPhysics.h"

USING_PTR(ComponentCollision);

//! @brief ヒット情報
struct Component::HitInfo
{
    bool                  hit_           = false;                 //!< ヒットしたか
    ComponentCollisionPtr collision_     = nullptr;               //!< 自分のコリジョン
    float3                push_          = {0.0f, 0.0f, 0.0f};    //!< めり込み量
    float3                hit_position_  = {0.0f, 0.0f, 0.0f};    //!< 当たった地点
    ComponentCollisionPtr hit_collision_ = nullptr;               //!< 当たったコリジョン
};

struct Component::HitInfoPhysics
{
    bool              hit_           = false;                 //!< ヒットしたか
    ComponentPhysics* collision_     = nullptr;               //!< 自分のコリジョン
    float3            push_          = {0.0f, 0.0f, 0.0f};    //!< めり込み量
    float3            hit_position_  = {0.0f, 0.0f, 0.0f};    //!< 当たった地点
    ComponentPhysics* hit_collision_ = nullptr;               //!< 当たったコリジョン
};
