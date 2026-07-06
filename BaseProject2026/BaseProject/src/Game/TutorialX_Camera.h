//---------------------------------------------------------------------------
//!	@file	TutorialX_Camera.h
//! @brief	TutorialX_Camera
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace TutorialX
{
	USING_PTR(Camera);
	class Camera : public Object
	{
	public:
		BP_OBJECT_DECL(Camera, "TutorialX::Camera");

		//! @brief 初期化
		//! @return 初期化終了
		bool Init() override;
	};
}	 // namespace TutorialX
