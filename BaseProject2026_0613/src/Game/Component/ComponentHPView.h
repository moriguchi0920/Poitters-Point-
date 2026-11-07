#pragma once
#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentPlayerState);

class ComponentHPView : public Component
{
public:
    BP_COMPONENT_DECL(ComponentHPView, u8"HP描画コンポーネント");

    //! @brief

    void Init() override;

    void Update() override;

    void PostDraw() override;

    //! @brief GUI
    //! GUIでのHPの確認や変更に使用
    void GUI() override;

    void SetPlayerID(int id) { player_id_ = id; }
    int  GetPlayerID() const { return player_id_; }

private:
    int HP_image_    = 0;
    int player_id_   = 0;
    int Heart_image_ = 0;
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentHPView, 1);
