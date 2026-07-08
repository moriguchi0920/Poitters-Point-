#pragma once

#include <System/Scene.h>

namespace Sample {

class SceneTest2 : public Scene::Base
{
public:
    BP_CLASS_DECL(SceneTest2, u8"ThirdPerson Gameテスト");

    bool Init() override;
    void Update() override;
    void Draw() override;
    void Exit() override;
    void GUI() override;
};

}    // namespace Sample
