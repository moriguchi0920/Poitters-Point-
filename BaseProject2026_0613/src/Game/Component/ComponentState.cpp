#pragma once
#include <Game/Component/ComponentState.h>

CEREAL_REGISTER_TYPE(ComponentState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentState)
