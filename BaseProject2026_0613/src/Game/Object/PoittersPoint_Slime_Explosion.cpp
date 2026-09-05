//---------------------------------------------------------------------------
//! @file   PoittersPoint_Slime_Explosion.cpp
//! @brief  PoittersPoint_Slime_Explosion
//---------------------------------------------------------------------------
#include "PoittersPoint_Slime_Explosion.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {

bool SlimeExplosion::Init()
{
    if(!Super::Init())
        return false;

    SetName("SlimeExplosion");

    // 爆発3Dモデルの設定
    auto model = AddComponent<ComponentModel>("data/Game/Models/Explosion/Explosion.mv1");
    if(model) {
        model->SetScaleAxisXYZ({1.0f, 1.0f, 1.0f});
    }

    // ★ 爆発SEの再生処理
    // DxLib標準関数の場合:
    PlaySoundFile("data/Game/Models/ExplosionSE/Explosion08-2(Short).mp3", DX_PLAYTYPE_BACK);

    return true;
}

void SlimeExplosion::Update()
{
    Super::Update();

    // 一定時間経過したらエフェクトを削除
    lifetime_ -= GetDeltaTime();
    if(lifetime_ <= 0.0f) {
        Scene::ReleaseObject(SharedThis());
    }
}

}    // namespace PoittersPoint
