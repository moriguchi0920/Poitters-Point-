//---------------------------------------------------------------------------
//!	@file	TutorialX_GameOver.h
//! @brief	TutorialX_GameOver
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace TutorialX
{

	class TutorialX_GameOver : public Scene::Base
	{
	public:
		BP_CLASS_DECL(TutorialX_GameOver, u8"TutorialXゲームオーバー");
		//! @brief 初期化
		//! @return 初期化済み
		bool Init() override;

		void Update() override;

		void LateDraw() override;

		void GUI() override;

	private:
		int x_	  = 20;
		int y_	  = 180;
		int size_ = 315;
	};

}	 // namespace TutorialX
