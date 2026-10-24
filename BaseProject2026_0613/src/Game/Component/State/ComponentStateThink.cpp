#pragma once
#include <Game/Component/State/ComponentState.h>
#include "ComponentStateThink.h"

CEREAL_REGISTER_TYPE(ComponentState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentState)

void ComponentStateThink::Init()
{
    __super::Init();
    // Init()はAddComponentされた翌フレームに走るため、ここで思考時間をリセットすると
    // AddComponent後にSetThinkingFinishTime()した値が消えてしまう
    // 初期値はメンバの初期値に任せる
}

void ComponentStateThink::Update()
{
    __super::Update();
    thinking_time += GetDeltaTime();
}

void ComponentStateThink::SetThinkingFinishTime(float time)
{
    thinking_finish_time = time;
}

float ComponentStateThink::GetCurThinkingTime()
{
    return thinking_time;
}

bool ComponentStateThink::GetFinished()
{
    return thinking_finish_time < thinking_time;
}
