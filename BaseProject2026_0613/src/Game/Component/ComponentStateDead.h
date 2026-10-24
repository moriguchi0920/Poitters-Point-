#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

USING_PTR(ComponentStateDead);

class ComponentStateDead : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateDead, u8"死亡状態コンポーネント");

    void Init() override;

    void Update() override;

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

CEREAL_CLASS_VERSION(ComponentStateDead, 1);
