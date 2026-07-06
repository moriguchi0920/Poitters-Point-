//---------------------------------------------------------------------------
//! @file   SceneVelocityBuffer.h
//! @brief  速度バッファサンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

//===========================================================================
//! サンプルシーン
//===========================================================================
class SceneVelocityBuffer final : public Scene::Base
{
public:
	BP_CLASS_DECL(SceneVelocityBuffer, u8"速度バッファ生成サンプル");

	virtual bool Init() override;	   //!< 初期化
	virtual void Update() override;	   //!< 更新
	virtual void Draw() override;	   //!< 描画
	virtual void Exit() override;	   //!< 終了
	virtual void GUI() override;	   //!< GUI表示

private:
};
