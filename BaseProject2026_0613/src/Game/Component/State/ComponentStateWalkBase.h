#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/State/ComponentState.h>

USING_PTR(ComponentStateWalkBase);

class ComponentStateWalkBase : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateWalkBase, u8"停止・歩きコンポーネント");

    void Init() override;

    void Update() override;

    ComponentStateWalkBasePtr SetMoveSpeed(const float speed);

    ComponentStateWalkBasePtr SetRotateSpeed(const float speed);

    const float GetMoveSpeed() const;
    const float GetRotateSpeed() const;

    inline const void  SetFrontRotate(float rotate) { front_rot_ = rotate; }
    inline const float GetFrontRotate() const { return front_rot_; }

    void GUI() override;

    void SetIsHolding(bool hold);

protected:
    float move_speed_ = 0.3f;
    float rot_speed_  = 20.0f;

    float front_rot_ = 0.0f;    //!<前方ベクトルの回転角度(0-360度)

    bool is_holding_;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(CEREAL_NVP(move_speed_), CEREAL_NVP(rot_speed_), CEREAL_NVP(front_rot_));

        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
    }
};

CEREAL_CLASS_VERSION(ComponentStateWalkBase, 1);
