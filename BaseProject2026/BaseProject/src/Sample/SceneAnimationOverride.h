//---------------------------------------------------------------------------
//! @file   SceneAnimationOverride.h
//! @brief  アニメーション上書き合成サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

//===========================================================================
//! アニメーション上書き合成サンプルシーン
//===========================================================================
class SceneAnimationOverride final : public Scene::Base
{
public:
	BP_CLASS_DECL(SceneAnimationOverride, u8"アニメーション上書き合成サンプル")

	bool Init() override;	   //!< 初期化
	void Update() override;	   //!< 更新
	void Draw() override;	   //!< 描画
	void Exit() override;	   //!< 終了
	void GUI() override;	   //!< GUI表示

private:
};
