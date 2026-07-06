//---------------------------------------------------------------------------
//!	@file	Tutorial_X.h
//! @brief	Tutorial_X
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace TutorialX
{

	class Tutorial_X : public Scene::Base
	{
	public:
		static constexpr int MAX_ENEMIES = 5;

		BP_CLASS_DECL(Tutorial_X, u8"TutorialX::Tutorial X のシーン");

		//! @brief 初期化
		//! @return 初期化済み
		bool Init() override;

		void Update() override;

		void AddDeadEnemy();

	private:
		void createEnemy();

		int enemy_dead_count_ = 0;
	};

}	 // namespace TutorialX
