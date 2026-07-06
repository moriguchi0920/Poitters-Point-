//---------------------------------------------------------------------------
//!	@file	TutorialX_Enemy.h
//! @brief	TutorialX_Enemy
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace TutorialX
{
	USING_PTR(Enemy);
	class Enemy : public Object
	{
	public:
		BP_OBJECT_DECL(Enemy, "TutorialX::Enemy");

		//! @brief 初期化
		//! @return 初期化終了
		bool Init() override;

		void Update() override;

		// 当たり判定が行われたときに呼ばれる関数
		void OnHit(const ComponentCollision::HitInfo& hit_info) override;

	private:
		bool is_dead_ = false;
	};
}	 // namespace TutorialX
