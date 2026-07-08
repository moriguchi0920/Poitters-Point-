#include <System/Scene.h>

namespace Sample ::GameSample {

class OverScene : public Scene::Base
{
public:
    BP_CLASS_DECL(OverScene, u8"GameSample/OverScene");

    bool Init() override;

    void Update() override;

    void Draw() override;
    /// <summary>
    ///
    /// </summary>
    void Exit() override;

    void GUI() override;
};

}    // namespace Sample::GameSample
