#pragma once

#include <System/Scene.h>

class SceneEmpty : public Scene::Base
{
public:
	BP_CLASS_DECL(SceneEmpty, u8"Empty　シーンテスト");

	bool Init() override;

	void Update() override;
};
