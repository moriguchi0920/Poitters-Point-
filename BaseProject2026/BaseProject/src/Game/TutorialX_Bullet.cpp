//---------------------------------------------------------------------------
//!	@file	TutorialX_Bullet.cpp
//! @brief	TutorialX_Bullet
//---------------------------------------------------------------------------

#include "TutorialX_Bullet.h"
#include "TutorialX.h"

namespace TutorialX
{
	//! @brief 初期化
	//! @return 初期化終了
	bool Bullet::Init()
	{
		Super::Init();

		SetName("Bullet");

		AddComponent<ComponentModel>("data/Sample/Coin/GoldCoin.mv1");

		auto col = AddComponent<ComponentCollisionSphere>()	   //
					   ->SetRadius(13.0);

		col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
		col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY |
								  (u32)ComponentCollision::CollisionGroup::GROUND |
								  (u32)ComponentCollision::CollisionGroup::ITEM);

		SetScaleAxisXYZ({0.2f});
		return true;
	}

	void Bullet::Update()
	{
		AddTranslate(direction_ * speed_);
	}

	void Bullet::SetDirection(float3 dir)
	{
		direction_ = normalize(dir);

		SetRotationToVector(direction_);
	}

	// 当たり判定が行われたときに呼ばれる関数
	void Bullet::OnHit(const ComponentCollision::HitInfo& hit_info)
	{
		// 自分を削除する
		Scene::Object::Release(SharedThis());

		auto mat = GetWorldMatrix();
		auto rot = matrix::rotateY(-0.5 * DX_PI);
		mat		 = mul(rot, mat);

		ComponentEffect::Object::Create("data/Sample/Effects/hit_eff.efkefc", mat);

		// 最後にこれを入れてください。ここでめり込みの解消を行っています。
		Super::OnHit(hit_info);
	}

}	 // namespace TutorialX
