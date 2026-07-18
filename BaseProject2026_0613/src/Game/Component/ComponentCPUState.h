#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentStateMachine.h>

USING_PTR(ComponentCPUState);

class ComponentCPUState : public ComponentStateMachine
{
public:
    BP_COMPONENT_DECL(ComponentCPUState, u8"CPU状態制御コンポーネント");

    void Init() override;

    void Update() override;

    void LateUpdate() override;

    void GUI() override;

    void GrabbableHit(bool is_hit_grabbable, ObjectPtr target);

private:
    ObjectWeakPtr grabbing_object_ptr_;

    bool can_grab_;
    bool can_throw_;
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentCPUState, 1);
