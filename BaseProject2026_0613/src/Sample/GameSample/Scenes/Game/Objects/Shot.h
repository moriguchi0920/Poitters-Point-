#pragma once

#include <System/Scene.h>
#include <System/Component/ComponentCollisionSphere.h>

namespace Sample::GameSample {

USING_PTR(Shot);

//! @brief 弾オブジェクト
class Shot : public Object
{
public:
    BP_OBJECT_DECL(Shot, u8"GameSample/Shot");

    //! @brief 発生用関数
    //! @param pos 発生場所
    //! @param vec 方向
    //! @return オブジェクト
    static ShotPtr Create(const float3& pos, const float3& vec);

    bool Init() override;

    void Update() override;

    void Draw() override;

    void OnHit(const ComponentCollision::HitInfo& hitInfo) override;

    void SetPositionAndDirection(const float3& pos, const float3& vec);

private:
    float3 vec_    = {0, 0, 0};    //< 進む方向
    float3 speed_  = 100.0f;       //< 1秒間に進む量
    float  radius_ = 1.0f;         //< 弾の大きさ
};

}    // namespace Sample::GameSample
