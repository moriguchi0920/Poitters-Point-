//---------------------------------------------------------------------------
//!	@file	TutorialX_Enemy.cpp
//! @brief	TutorialX_Enemy
//---------------------------------------------------------------------------

#include "TutorialX_Enemy.h"
#include "TutorialX.h"

namespace TutorialX
{
	//int randomRange(int start, int end)
	//{
	//	return start + rand() % ((end - start) + 1);
	//}

	//! @brief 初期化
	//! @return 初期化終了
	bool Enemy::Init()
	{
		Super::Init();

		SetName("Enemy");

		//auto Enemy = Scene::Object::Create<Object>("Enemy");
		auto model = AddComponent<ComponentModel>("data/Game/Models/Enemy/Enemy.mv1");
		model->SetAnimation({
			{"idle",	 "data/Game/Models/Enemy/Anims/Idle.mv1", 0, 1.0f}, // Idle
			{"walk",	 "data/Game/Models/Enemy/Anims/Walk.mv1", 0, 1.0f}, // Jump
			{"dead", "data/Game/Models/Enemy/Anims/Dead.mv1", 0, 1.0f}, // Dead
		});
		model->PlayAnimationNoSame("walk", true);

		auto randomRange = [](int start, int end) -> int
		{
			return start + rand() % ((end - start) + 1);
		};
		SetTranslate({randomRange(-150, 150), 50, randomRange(-150, 150)});

		AddComponent<ComponentCollisionCapsule>()	 //
			->SetRadius(3.0f)
			->SetHeight(13.0f)
			->SetCollisionGroup(ComponentCollision::CollisionGroup::ENEMY)
			->UseGravity();

		return true;
	}

	void Enemy::Update()
	{
		Super::Update();

		if(!is_dead_)
		{
			//-------------------------------------------------------------------------------
			// 敵の方向をプレイヤーのほうに向ける　④
			//-------------------------------------------------------------------------------
			auto player = Scene::Object::Get<Object>("Player");

			// プレイヤーが存在している時のみ
			if(player)
			{
				// プレイヤーの方向に、敵の向きを最大3度回転させます
				SetRotationToPositionWithLimit(player->GetTranslate(), 3.0f);

				AddTranslate({0, 0, -0.1}, true);
			}
			//-------------------------------------------------------------------------------
		}
	}

	// 当たり判定が行われたときに呼ばれる関数
	void Enemy::OnHit(const ComponentCollision::HitInfo& hit_info)
	{
		// 自分を削除する
		//Scene::Object::Release(SharedThis());

		auto name = hit_info.hit_collision_->GetOwner()->GetNameDefault();
		if(name == "Bullet")
		{
			auto now_scene = Scene::GetCurrentScene();
			if(auto scene = dynamic_cast<Tutorial_X*>(now_scene))
			{
				scene->AddDeadEnemy();

				if(auto model = GetComponent<ComponentModel>())
					model->PlayAnimationNoSame("dead");
				//if(auto col = GetComponent<ComponentCollisionCapsule>())
				//	RemoveComponent(col);
				RemoveComponent<ComponentCollisionCapsule>();
			}
			is_dead_ = true;

			//if(auto model= Get)

			//Scene::Object::Release(SharedThis());
		}
		// 何と当たったかを確認する
		//printfDx("%s\n", name.data());

		// 最後にこれを入れてください。ここでめり込みの解消を行っています。
		Super::OnHit(hit_info);
	}

}	 // namespace TutorialX
