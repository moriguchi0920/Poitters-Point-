//---------------------------------------------------------------------------
//!	@file	TutorialX_Title.cpp
//! @brief	TutorialX_Title
//---------------------------------------------------------------------------
#include "TutorialX_Title.h"
#include "TutorialX.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentObjectController.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentSpringArm.h>

namespace TutorialX
{

	bool TutorialX_Title::Init()
	{
		// 最初に1回動作する
		// ただし trueを返さなければ Initに何回も来る仕様。
		auto camera = Scene::Object::Create<Object>("Camera");

		camera->AddComponent<ComponentCamera>()	   //
			->SetPositionAndTarget({0, 20, -50}, {0, 10, 0});

		auto ground = Scene::Object::Create<Object>("Ground");
		ground->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
		ground->AddComponent<ComponentCollisionModel>()
			->AttachToModel();
		return true;
	}

	void TutorialX_Title::Update()
	{
		if(Input::IsKeyDown(KEY_INPUT_SPACE))
			Scene::Change(Scene::GetScene<Tutorial_X>());

		if(GetSceneTime() >= 10.0f)
			Scene::Change(Scene::GetScene<Tutorial_X>());
	}

	void TutorialX_Title::LateDraw()
	{
		float ADJUST_X = (WINDOW_W / 1280.0f);
		float ADJUST_Y = (WINDOW_H / 720.0f);

		SetFontSize(static_cast<int>(title_size_ * ADJUST_X));

		DrawFormatString(
			static_cast<int>(title_x_ * ADJUST_X),
			static_cast<int>(title_y_ * ADJUST_Y),
			GetColor(255, 255, 255),
			"Title");
	}

	void TutorialX_Title::GUI()
	{
		ImGui::DragInt("TitleLeft", &title_x_);
		ImGui::DragInt("TitleTop", &title_y_);
		ImGui::DragInt("TitleSize", &title_size_);
	}

}	 // namespace TutorialX
