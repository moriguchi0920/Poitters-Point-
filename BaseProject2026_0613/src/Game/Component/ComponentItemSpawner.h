#pragma once

#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentItemSpawner);

class ComponentItemSpawner : public Component
{
public:
    BP_COMPONENT_DECL(ComponentItemSpawner, u8"アイテムスポナーコンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    void SpawnItem();

private:
    ObjectWeakPtr Spawn_Item_ptr_;

    // タイマー管理用変数
    float spawn_timer_      = 0.0f;    // カウントダウンタイマー
    float spawn_delay_time_ = 5.0f;    // アイテムの再生成にかかる時間

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ・ロード
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
        arc(cereal::make_nvp("spawn_delay_time_", spawn_delay_time_));
    }
    //@}
};

CEREAL_CLASS_VERSION(ComponentItemSpawner, 1);
