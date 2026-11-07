#pragma once
#include <System/Scene.h>
#include <Game/Component/ComponentHPView.h>

namespace PoittersPoint {
USING_PTR(HPUI);
class HPUI : public Object
{
public:
    BP_OBJECT_DECL(HPUI, "PoittersPoint::HPUI");

    bool Init() override
    {
        __super::Init();
        AddComponent<ComponentHPView>();
        return true;
    }
};
}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::HPUI)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::HPUI)
