//---------------------------------------------------------------------------
//!	@file	Tutorial_X.cpp
//! @brief	Tutorial_X
//---------------------------------------------------------------------------
#include "TutorialX.h"
#include "TutorialX_GameOver.h"

#include "TutorialX_Player.h"
#include "TutorialX_Ground.h"
#include "TutorialX_Camera.h"
#include "TutorialX_Enemy.h"

namespace TutorialX
{

	void Tutorial_X::createEnemy()
	{
		for(int i = 0; i < MAX_ENEMIES; i++)
			Scene::Object::Create<Enemy>();
	}

	//! @brief 初期化
	//! @return 初期化済み
	bool Tutorial_X::Init()
	{
		// 最初に1回動作する
		// ただし trueを返さなければ Initに何回も来る仕様。

		Scene::Object::Create<Ground>();

		Scene::Object::Create<Player>();

		Scene::Object::Create<Camera>();

		createEnemy();

		return true;
	}

	void Tutorial_X::Update()
	{
		printfDx("\nDEAD ENEMY: %d", enemy_dead_count_);

		if(enemy_dead_count_ >= MAX_ENEMIES)
		{
			//createEnemy();
			Scene::Change(Scene::GetScene<TutorialX_GameOver>());

			enemy_dead_count_ = 0;
		}
	}

	void Tutorial_X::AddDeadEnemy()
	{
		enemy_dead_count_++;
	}

}	 // namespace TutorialX
