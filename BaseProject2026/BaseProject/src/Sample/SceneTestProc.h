#pragma once

#include <System/Scene.h>

class SceneTestProc : public Scene::Base
{
public:
	BP_CLASS_DECL(SceneTestProc, u8"プロセステスト");

	bool Init() override;
	void Update() override;
	void Draw() override;
	void Exit() override;
	void GUI() override;

	void InitSerialize() override;
};
