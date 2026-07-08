#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Object/PoittersPoint_Bullet.h>

USING_PTR(ComponentStateThrow);

class ComponentStateThrow : public Component
{
public:
    BP_COMPONENT_DECL(ComponentStateThrow, u8"投擲攻撃コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

private:
    PoittersPoint::BulletWeakPtr bullet_;

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
