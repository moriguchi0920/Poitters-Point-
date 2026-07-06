//---------------------------------------------------------------------------
//!	@file	TutorialX_Bullet.h
//! @brief	TutorialX_Bullet
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace TutorialX
{
	USING_PTR(Bullet);
	class Bullet : public Object
	{
	public:
		BP_OBJECT_DECL(Bullet, "TutorialX::Bullet");

		//! @brief 初期化
		//! @return 初期化終了
		bool Init() override;

		void Update() override;

		void SetDirection(float3 dir);

		// 当たり判定が行われたときに呼ばれる関数
		void OnHit(const ComponentCollision::HitInfo& hit_info) override;

	private:
		float3 direction_ = {0, 1, 0};
		float  speed_	  = 2.0f;
	};
}	 // namespace TutorialX
