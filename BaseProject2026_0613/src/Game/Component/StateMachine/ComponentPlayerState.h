#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/StateMachine/ComponentStateMachine.h>

USING_PTR(ComponentPlayerState);

namespace PoittersPoint {
class Player;
}

class ComponentPlayerState : public ComponentStateMachine
{
public:
    friend class PoittersPoint::Player;

    BP_COMPONENT_DECL(ComponentPlayerState, u8"プレイヤー状態制御コンポーネント");

    void Init() override;

    void Update() override;

    void LateUpdate() override;

    void GUI() override;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentPlayerState, 1);
