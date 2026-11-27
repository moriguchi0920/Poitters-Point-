//---------------------------------------------------------------------------
//! @file   PoittersPoint_Slime.cpp
//! @brief  PoittersPoint_Slime (投げ物：スライム。投げて何かに当たると爆発する)
//---------------------------------------------------------------------------
#include "PoittersPoint_Slime.h"
#include "PoittersPoint_Slime_Explosion.h"    // SlimeExplosionの型定義を読み込む
#include "Game/Component/ComponentGrabbable.h"
#include "Game/Component/ComponentHitPoints.h"
#include "Game/Component/ComponentStateMachine.h"
#include "Game/Component/ComponentCharacterPhysics.h"
#include "Game/System/PoittersPoint_PhysicsHelper.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentPhysics.h>

namespace PoittersPoint {

bool Slime::Init()
{
    if(!Super::Init())
        return false;

    SetName("Slime");

    // 物理ボディは作成時の位置に置かれるため、先に位置を決めておく
    SetTranslate({0.0f, 6.0f, 12.0f});

    // 3Dモデルの設定
    // 物理ボディの行列にはスケールが無く、PostPhysics でオブジェクトの行列が上書きされるため
    // 大きさはオブジェクトではなくモデル側に持たせる
    // (ComponentModel の既定スケールは 0.1。以前はオブジェクトに 0.2 を掛けていたので合計 0.02 倍)
    auto model = AddComponent<ComponentModel>("data/Game/Models/Slime/Slime.mv1");
    model->SetScaleAxisXYZ(model->GetScaleAxisXYZ() * 0.2f);

    // 柔らかく弾む球(モデルの大きさに合わせる)
    auto physics = AddComponent<ComponentPhysics>();
    radius_      = PhysicsHelper::CreateSphereFitModel(physics, model, radius_, density_);
    physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Actived, true);
    physics->SetPhysicsReal(1.0f);
    physics->SetCollisionType(ComponentPhysics::CollisionType::ITEM);
    physics->SetFriction(0.6f);
    physics->SetRestitution(0.25f);    //!< 少し弾む(大きすぎると静止中も細かく跳ね続ける)
    physics->SetLinearDamping(2.0f);
    physics->SetAngularDamping(2.0f);

    // 持ち上げ機能（Grabbable）
    auto grabbable = AddComponent<ComponentGrabbable>();
    grabbable->SetLiftTime(1.0f);
    grabbable->SetDamage(2.0f);

    // 投げられて何かに当たったら爆発する(物理コールバック中にオブジェクトを消さないよう、次の Update で爆発させる)
    grabbable->OnImpactFunc = [this](Object*) { explode_requested_ = true; };

    return true;
}

void Slime::Update()
{
    Super::Update();

    if(explode_requested_) {
        explode_requested_ = false;
        Explode();
    }
}

void Slime::GUI()
{
    Super::GUI();

    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("Slime")) {
            ImGui::DragFloat(u8"爆発範囲", &explosion_radius_, 1.0f, 0.0f, 500.0f);
            ImGui::DragFloat(u8"爆発ダメージ", &explosion_damage_, 0.1f, 0.0f, 999.0f);
            ImGui::DragFloat(u8"爆風の強さ", &explosion_power_, 1.0f, 0.0f, 1000.0f);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//===========================================================================
//! @brief 爆発処理：範囲内のHPを持つ相手にダメージ + 外向きに吹き飛ばす
//===========================================================================
void Slime::Explode()
{
    const float3 my_pos = GetTranslate();

    for(auto& obj : Scene::Object::GetArray<Object>()) {
        if(!obj || obj.get() == this)
            continue;

        auto hp = obj->GetComponent<ComponentHitPoints>();
        if(!hp)
            continue;

        float3 vec      = obj->GetTranslate() - my_pos;
        float  distance = (float)length(vec);
        if(distance > explosion_radius_)
            continue;

        hp->TakeDamage(explosion_damage_);

        // 爆心から外向き + 上向きに吹き飛ばす(近いほど強い)
        float3 dir      = {vec.x, 0.0f, vec.z};
        dir             = (float)length(dir) > 0.0001f ? normalize(dir) : float3(0.0f, 0.0f, 1.0f);
        float  strength = explosion_power_ * (1.0f - 0.5f * distance / std::max(explosion_radius_, 0.001f));
        float3 knock    = dir * strength + float3(0.0f, strength * 0.6f, 0.0f);

        if(auto state_machine = obj->GetComponent<ComponentStateMachine>())
            state_machine->KnockDown(knock);
        else if(auto mover = obj->GetComponent<ComponentCharacterPhysics>())
            mover->Launch(knock);
    }

    // 爆発エフェクトの生成
    if(auto explosion = Scene::Object::Create<SlimeExplosion>())
        explosion->SetTranslate(my_pos);

    // スライム自身の削除
    Scene::Object::Release(SharedThis());
}

}    // namespace PoittersPoint
