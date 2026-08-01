#pragma once

#include <System/Scene.h>
#include <System/Component/Component.h>
#include <string>

USING_PTR(ComponentItemSpawner);

class ComponentItemSpawner : public Component
{
public:
    BP_COMPONENT_DECL(ComponentItemSpawner, u8"アイテムスポナーコンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    void SpawnItem();

    void LoadModel();

private:
    ObjectWeakPtr Spawn_Item_ptr_;

    // タイマー管理用変数
    float spawn_timer_      = 0.0f;    // カウントダウンタイマー
    float spawn_delay_time_ = 5.0f;    // アイテムの再生成にかかる時間

    // スポナー本体のモデルパス
    std::string model_path_ = "data/Game/Models/ItemSpawner/Dragon.mv1";

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
        arc(cereal::make_nvp("model_path_", model_path_));
    }
    //@}
};

CEREAL_CLASS_VERSION(ComponentItemSpawner, 1);
