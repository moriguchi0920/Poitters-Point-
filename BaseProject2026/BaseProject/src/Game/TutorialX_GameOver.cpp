//---------------------------------------------------------------------------
//!	@file	TutorialX_GameOver.cpp
//! @brief	TutorialX_GameOver
//---------------------------------------------------------------------------
#include "TutorialX_GameOver.h"
#include "TutorialX.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentObjectController.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentSpringArm.h>
#include "TutorialX_Title.h"

namespace TutorialX
{

	bool TutorialX_GameOver::Init()
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

	void TutorialX_GameOver::Update()
	{
		//if(Input::IsKeyDown(KEY_INPUT_SPACE))
		//	Scene::Change(Scene::GetScene<Tutorial_X>());

		if(GetSceneTime() >= 3.0f)
			Scene::Change(Scene::GetScene<TutorialX_Title>());
	}

	void TutorialX_GameOver::LateDraw()
	{
		float ADJUST_X = (WINDOW_W / 1280.0f);
		float ADJUST_Y = (WINDOW_H / 720.0f);

		SetFontSize(static_cast<int>(size_ * ADJUST_X));

		DrawFormatString(
			static_cast<int>(x_ * ADJUST_X),
			static_cast<int>(y_ * ADJUST_Y),
			GetColor(255, 255, 255),
			"GameOver");
	}

	void TutorialX_GameOver::GUI()
	{
		ImGui::DragInt("Left", &x_);
		ImGui::DragInt("Top", &y_);
		ImGui::DragInt("Size", &size_);
	}

}	 // namespace TutorialX
