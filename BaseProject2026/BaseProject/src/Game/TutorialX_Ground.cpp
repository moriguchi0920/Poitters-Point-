//---------------------------------------------------------------------------
//!	@file	TutorialX_Player.cpp
//! @brief	TutorialX_Player
//---------------------------------------------------------------------------

#include "TutorialX_Ground.h"
#include "TutorialX.h"

namespace TutorialX
{
	//! @brief 初期化
	//! @return 初期化終了
	bool Ground::Init()
	{
		Super::Init();

		SetName("Ground");

		//auto ground = Scene::Object::Create<Object>("Ground");
		AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
		AddComponent<ComponentCollisionModel>()->AttachToModel();

		return true;
	}
}	 // namespace TutorialX
