#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentHitPoints);

class ComponentHitPoints : public Component
{
public:
    BP_COMPONENT_DECL(ComponentHitPoints, u8"ヒットポイント(HP)コンポーネント");

    //! @brief 初期化
    void Init() override;

    //! @brief ヒットポイントを取得する
    //! @return ヒットポイント
    float GetHitPoints();

    //! @brief 最大ヒットポイントを取得する
    //! @return 最大ヒットポイント
    float GetMaxHitPoints() const;

    //! @brief HPが尽きているか
    //! @retval true  HPが0以下
    //! @retval false HPが残っている
    bool IsDead() const;

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
    float hit_points_ = 5.0f;

    // 最大ヒットポイント
    // ※以前はstaticだったため全キャラクターで共有されていた(Enemyで2にするとPlayerも2になる)
    //   コンポーネントごとに持つようにした
    float max_hit_points_ = 5.0f;

    // SetMax～で値が設定済みか(設定済みならInitで現在HPを上書きしない)
    bool is_hp_set_ = false;

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
