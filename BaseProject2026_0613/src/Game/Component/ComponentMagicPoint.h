#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentPlayerState);

class ComponentMagicPoint : public Component
{
public:
    BP_COMPONENT_DECL(ComponentMagicPoint, u8"マジックポイント(MP)コンポーネント");

    //! @brief

    void Init() override;

    //! @brief マジックポイントを取得する
    //! @return マジックポイント
    float GetMagicPoints();

    //! @brief 引数の消費量でMPを減算する
    //! @param cost
    void UseMP(float cost);

    //! @brief 最大MPを設定
    //! @param new_max_mp
    void SetMaxMP(float new_max_mp);

    //! @brief 最大MPと現在のMPを設定
    //! @param new_mp
    void SetMaxAndCurrentMP(float new_mp);

    //! @brief 最大MPと現在のMPをそれぞれ設定
    //! @param new_max_mp
    //! @param new_mp
    void SetMaxAndCurrentMP(float new_max_mp, float new_mp);

    //! @brief GUI
    //! GUIでのMPの確認や変更に使用
    void GUI() override;

private:
    // マジックポイント
    float magic_points_;

    // 最大マジックポイント
    static float max_magic_points_;

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentMagicPoint, 1);
