#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentPlayerState);

class ComponentHitPoints : public Component
{
public:
    BP_COMPONENT_DECL(ComponentHitPoints, u8"ヒットポイント(HP)コンポーネント");

    //! @brief

    void Init() override;

    //! @brief ヒットポイントを取得する
    //! @return ヒットポイント
    float GetHitPoints();

    //! @brief 引数のダメージでHPを減算する
    //! @param damage
    void TakeDamage(float damage);

    //! @brief 最大HPを設定
    //! @param new_max_hp
    void SetMaxHP(float new_max_hp);

    //! @brief 最大HPと現在のHPを設定
    //! @param new_hp
    void SetMaxAndCurrentHP(float new_hp);

    //! @brief 最大HPと現在のHPをそれぞれ設定
    //! @param new_max_hp
    //! @param new_current_hp
    void SetMaxAndCurrentHP(float new_max_hp, float new_hp);

    //! @brief GUI
    //! GUIでのHPの確認や変更に使用
    void GUI() override;

private:
    // ヒットポイント
    float hit_points_;

    // 最大ヒットポイント
    static float max_hit_points_;

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentHitPoints, 1);
