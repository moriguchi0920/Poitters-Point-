//---------------------------------------------------------------------------
//!	@file	TutorialX_Ground.h
//! @brief	TutorialX_Ground
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace TutorialX
{
	USING_PTR(Ground);
	class Ground : public Object
	{
	public:
		BP_OBJECT_DECL(Ground, "TutorialX::Ground");

		//! @brief 初期化
		//! @return 初期化終了
		bool Init() override;
	};
}	 // namespace TutorialX
