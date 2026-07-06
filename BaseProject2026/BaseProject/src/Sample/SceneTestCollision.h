#pragma once

#include <System/Scene.h>

class SceneTestCollision : public Scene::Base
{
public:
	BP_CLASS_DECL(SceneTestCollision, u8"TestCollision");

	bool Init() override;
	void Update() override;
	void Draw() override;
	void Exit() override;
	void GUI() override;
};
