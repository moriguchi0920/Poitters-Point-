#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

USING_PTR(ComponentStateThrow);

class ComponentStateThrow : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateThrow, u8"投擲攻撃コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    void SetThrowObject(ObjectWeakPtr object_ptr);

    //! 空中投げパラメータの設定
    void SetInAir(bool in_air, float current_velocity, float ground_y, float gravity)
    {
        is_in_air_        = in_air;
        current_velocity_ = current_velocity;
        ground_y_         = ground_y;
        gravity_          = gravity;
    }

private:
    ObjectWeakPtr throw_weak_ptr_;

    // 空中投げ用パラメータ
    bool  is_in_air_        = false;
    float current_velocity_ = 0.0f;
    float ground_y_         = 0.0f;
    float gravity_          = 0.05f;
    bool  is_released_      = false;    // 手離しが完了したか

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentStateThrow, 1);
