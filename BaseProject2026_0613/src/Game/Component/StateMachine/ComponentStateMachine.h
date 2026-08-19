#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/State/ComponentState.h>
USING_PTR(ComponentStateMachine);

class Component;

class ComponentStateMachine : public Component
{
public:
    BP_COMPONENT_DECL(ComponentStateMachine, u8"ステートマシンコンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    template <class T>
    T* ChangeState()
    {
        static_assert(std::derived_from<T, ComponentState>, "ステートコンポーネント以外は指定できません");
        if(auto state = GetOwner()->GetComponent<ComponentState>()) {
            return state->ChangeState<T>();
        }
    }

    const std::string GetStateName() const;

    void GrabbableHit(ObjectPtr target);

    bool GetCanGrab();

protected:
    bool          can_grab_;
    bool          can_throw_;
    ObjectWeakPtr grabbing_object_ptr_;

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

CEREAL_CLASS_VERSION(ComponentStateMachine, 1);
