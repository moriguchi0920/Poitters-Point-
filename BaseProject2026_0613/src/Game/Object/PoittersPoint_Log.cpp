//---------------------------------------------------------------------------
//! @file   PoittersPoint_Log.cpp
//! @brief  PoittersPoint_Log (投げ物：丸太)
//! @details 円柱の剛体として物理に乗せる。投げると軸が進行方向と直交する向きで
//!          前転しながら飛ぶため、着地後はそのまま地面を転がっていく。
//---------------------------------------------------------------------------
#include "PoittersPoint_Log.h"
#include "Game/Component/ComponentGrabbable.h"
#include "Game/System/PoittersPoint_PhysicsHelper.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentPhysics.h>
#include <System/Physics/RigidBody.h>

namespace PoittersPoint {

bool Log::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();

    SetName("Log");

    // 物理ボディは作成時の位置に置かれるため、先に位置を決めておく
    SetTranslate({10.0f, 10.0f, -20.0f});
    SetScaleAxisXYZ({1.0f});

    auto model = AddComponent<ComponentModel>("data/Game/Models/Log/Log.mv1");

    // 丸太の長軸に合わせた円柱(転がる形)。モデルが取得できない時は従来のカプセル寸法で作る
    auto physics = AddComponent<ComponentPhysics>();
    if(!PhysicsHelper::CreateCylinderFitModel(physics, model, density_, radius_, half_length_)) {
        physics->CreateCylinder(half_length_, radius_, density_);
    }
    physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Actived, true);
    physics->SetPhysicsReal(1.0f);
    physics->SetCollisionType(ComponentPhysics::CollisionType::ITEM);
    physics->SetFriction(0.8f);           //!< 滑らずに転がるよう摩擦は高め
    physics->SetRestitution(0.05f);       //!< 木なのでほとんど跳ねない
    physics->SetLinearDamping(0.8f);     //!< 接地中の減速は弱め(岩よりずっとよく転がる)
    physics->SetAngularDamping(0.6f);    //!< 転がり抵抗も弱め

    auto grabbable = AddComponent<ComponentGrabbable>();
    grabbable->SetLiftTime(1.0f);
    grabbable->SetDamage(2.0f);
    grabbable->SetKnockbackPower(160.0f);    // 重いので大きく吹き飛ばす
    grabbable->SetThrowSpeedScale(0.8f);     // 重いので少し遅い
    grabbable->SetAlignRollAxisOnThrow(true);
    grabbable->SetRollSpin(1.0f, radius_);    // 着地後そのまま転がる回転

    // SE
    se_handle_ = LoadSoundMem("data/Game/LogSE/Log1.mp3");

    return true;
}

void Log::Update()
{
    __super::Update();

    if(se_cool_down_ > 0.0f)
        se_cool_down_ -= GetDeltaTime();
}

void Log::PrePhysics()
{
    __super::PrePhysics();

    // 着地の勢いを判定するため、物理ステップ前の速度を覚えておく
    if(auto physics = GetComponent<ComponentPhysics>()) {
        if(auto body = physics->GetRigidBody())
            velocity_before_step_ = body->linearVelocity();
    }
}

void Log::OnHitPhysics(const ComponentPhysics::HitInfo& hit_info)
{
    __super::OnHitPhysics(hit_info);

    if(!hit_info.hit_collision_ || se_cool_down_ > 0.0f || se_handle_ == -1)
        return;

    // 地面・壁に勢いよくぶつかった時だけ鳴らす
    auto type = hit_info.hit_collision_->GetCollisionType();
    if(type != ComponentPhysics::CollisionType::GROUND && type != ComponentPhysics::CollisionType::WALL)
        return;

    if((float)length(velocity_before_step_) < se_min_speed_)
        return;

    PlaySoundMem(se_handle_, DX_PLAYTYPE_BACK);
    se_cool_down_ = 0.3f;    // 連続再生防止
}

void Log::Exit()
{
    if(se_handle_ != -1) {
        DeleteSoundMem(se_handle_);
        se_handle_ = -1;
    }
    __super::Exit();
}

void Log::GUI()
{
    __super::GUI();

    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("Log")) {
            ImGui::Text(u8"円柱 半径: %.2f / 長さ: %.2f", radius_, half_length_ * 2.0f);
            ImGui::DragFloat(u8"SEが鳴る速さ", &se_min_speed_, 1.0f, 0.0f, 500.0f);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

}    // namespace PoittersPoint
