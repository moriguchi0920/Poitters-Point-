//---------------------------------------------------------------------------
//! @file   PoittersPoint_Bullet.cpp
//! @brief  PoittersPoint_Bullet
//---------------------------------------------------------------------------
#include "PoittersPoint_Bullet.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {
// namespace PoittersPoint

// 弾の存在時間(秒)[全部の弾の共通]
float Bullet::max_life_          = 5.0f;
float Bullet::max_speed_         = 2.0f;
float Bullet::max_rotateX_speed_ = 0.1f;

bool Bullet::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetName("Bullet");
    AddComponent<ComponentModel>("data/Sample/Coin/GoldCoin.mv1");

    auto col = AddComponent<ComponentCollisionSphere>()->SetRadius(3.0f);

    col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                              (u32)ComponentCollision::CollisionGroup::ITEM);

    //AddComponent<ComponentCollisionModel>();
    //if(auto collision = GetComponent<ComponentCollisionModel>()) {
    //    collision->AttachToModel();    // コリジョンをモデルに合わせる
    //}

    SetScaleAxisXYZ({0.2f});

    speed_ = max_speed_;    // 共通のスピードを設定する
    life_  = max_life_;     // 生存時間の設定をする

    return true;
}

void Bullet::Update()
{
    __super::Update();

    // 弾を移動させる
    AddTranslate(direction_ * speed_);

    // 弾を進行方向(自身の所持している向き)に向かせる
    SetRotationToVector(direction_);

    // 回転角度の更新
    //spin_angle_ += max_rotateX_speed_ * GetDeltaTime();
    spin_angle_ += max_rotateX_speed_;

    // X軸回転行列を作成
    auto rot_x = matrix::rotateX(spin_angle_);

    // 姿勢行列を取得
    auto mat = GetMatrix();

    // 今の姿勢行列に回転行列を掛け合わせる
    SetMatrix(mul(rot_x, mat));

    //--------------------------------------------------
    // 生存時間のチェック
    //--------------------------------------------------
    // 生存時間を減らす
    life_ -= GetDeltaTime();

    if(life_ <= 0.0f) {
        // 生存時間がなくなったら消滅させる
        Scene::Object::Release(SharedThis());
    }
}

void Bullet::GUI()
{
    __super::GUI();
}

void Bullet::SetDirection(float3 dir)
{
    direction_ = normalize(dir);

    SetRotationToVector(direction_);
}

void Bullet::ResetDirection()
{
    SetRotationToVector(direction_);
}

void Bullet::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    /*
    // 当たった相手の名前がEnemyだったら消去する
    auto name = hit_info.hit_collision_->GetOwner()->GetNameDefault();
    if(name == "Enemy") {
        hit_info.collision_->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, true);
        // 自分を削除する
        Scene::Object::Release(SharedThis());
    }
    */

    // 自分を削除する
    Scene::Object::Release(SharedThis());

    hit_info.collision_->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, true);

    auto mat = GetWorldMatrix();
    auto rot = matrix::rotateY(-0.5 * DX_PI);

    mat = mul(rot, mat);

    // エフェクト
    //ComponentEffect::Object::Create("data/Sample/Effects/hit_eff.efkefc", GetTranslate(), rot);
    ComponentEffect::Object::Create("data/Sample/Effects/hit_eff.efkefc", mat);

    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
