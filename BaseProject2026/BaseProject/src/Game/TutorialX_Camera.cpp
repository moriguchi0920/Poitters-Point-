//---------------------------------------------------------------------------
//!	@file	TutorialX_Camera.cpp
//! @brief	TutorialX_Camera
//---------------------------------------------------------------------------

#include "TutorialX_Camera.h"
#include "TutorialX.h"

namespace TutorialX
{
	//! @brief 初期化
	//! @return 初期化終了
	bool Camera::Init()
	{
		Super::Init();

		SetName("Camera");

		//auto camera = Scene::Object::Create<Object>("Camera");
		AddComponent<ComponentCamera>()	   //
			->SetPositionAndTarget({0, 20, -50}, {0, 10, 0});
		AddComponent<ComponentSpringArm>()->SetSpringArmObject("Player");

		return true;
	}
}	 // namespace TutorialX
