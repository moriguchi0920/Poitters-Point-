#pragma once

#include <System/Scene.h>
#include <System/Component/ComponentCollisionSphere.h>

namespace Sample::GameSample {

USING_PTR(Box);

class Box : public Object
{
public:
    BP_OBJECT_DECL(Box, u8"GameSample/Box");

    //! @brief 発生用関数
    //! @param pos 発生場所
    //! @param front 方向
    //! @return オブジェクト
    static BoxPtr Create(const float3& pos, const float3& front = {0, 0, 1});

    bool Init() override;

    void Update() override;
};

}    // namespace Sample::GameSample
