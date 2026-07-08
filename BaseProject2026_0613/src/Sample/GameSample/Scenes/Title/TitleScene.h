#include <System/Scene.h>

namespace Sample::GameSample {

class TitleScene : public Scene::Base
{
public:
    BP_CLASS_DECL(TitleScene, u8"GameSample/TitleScene");

    std::string Name() { return "GameSample/TitleScene"; }

    bool Init() override;

    void Update() override;

    void Draw() override;

    void Exit() override;

    void GUI() override;
};

}    // namespace Sample::GameSample
