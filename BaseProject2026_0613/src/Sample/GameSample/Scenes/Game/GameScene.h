#include <System/Scene.h>

namespace Sample::GameSample {

class GameScene : public Scene::Base
{
public:
    BP_CLASS_DECL(GameScene, u8"GameSample/GameScene");

    std::string Name() { return "GameSample/GameScene"; }

    bool Init() override;

    void Update() override;

    void Draw() override;

    void Exit() override;

    void GUI() override;
};

}    // namespace Sample::GameSample
