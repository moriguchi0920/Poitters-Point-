#pragma once

#include <System/Scene.h>

//! @brief シーン
class SceneObjectTracking : public Scene::Base
{
public:
	BP_CLASS_DECL(SceneObjectTracking, u8"Object Tracking 使用サンプル");

	//! @brief シーン初期化関数を継承します
	//! @return シーン初期化が終わったらtrueを返します
	bool Init() override;

	//! @brief 描画関数
	void Draw() override;

	//! @brief 終了処理
	void Exit() override;
};
