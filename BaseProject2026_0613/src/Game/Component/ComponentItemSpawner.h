#pragma once

#include <System/Scene.h>
#include <System/Component/Component.h>
#include <string>
#include <vector>

USING_PTR(ComponentItemSpawner);

//! @brief 生成可能なアイテムの種類
enum class ItemType
{
    None,       // 補充対象外（固定配置の岩など）
    Crate,      // 木箱
    Slime,      // スライム
    Missile,    // ミサイル
    // 必要に応じて追加
};

//! @brief スポナー地点の情報構造体
struct SpawnerPoint
{
    float3        position{0.0f, 0.0f, 0.0f};         // 地点の座標
    ObjectWeakPtr current_item_ptr;                   // 現在配置されているアイテムの参照
    ItemType      assigned_type = ItemType::Crate;    // ★このマスに降らせる固定のアイテム種類

    //! @brief その場所にアイテムが無い（破棄/消滅された）か判定（補充対象外でないことも確認）
    bool IsEmptyAndNeedsSpawn() const { return assigned_type != ItemType::None && current_item_ptr.expired(); }
};

class ComponentItemSpawner : public Component
{
public:
    BP_COMPONENT_DECL(ComponentItemSpawner, u8"アイテムスポナーコンポーネント");

    void Init() override;
    void Update() override;
    void GUI() override;

    void SpawnItemAt(int point_index);
    void LoadModel();

private:
    //! @brief 補充が必要な空き地点を1つ取得する
    int GetEmptyPointIndex();

    //! @brief 指定した種類のアイテムインスタンスを生成するファクトリ関数
    ObjectPtr CreateItemInstance(ItemType type);

private:
    std::vector<SpawnerPoint> spawner_points_;
    int                       target_point_index_ = -1;

    float move_speed_       = 90.0f;
    float float_timer_      = 0.0f;
    float spawn_timer_      = 0.0f;
    float spawn_delay_time_ = 0.3f;

    std::string model_path_ = "data/Game/Models/ItemSpawner/Dragon.mv1";

    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
        arc(cereal::make_nvp("spawn_delay_time_", spawn_delay_time_));
        arc(cereal::make_nvp("move_speed_", move_speed_));
        arc(cereal::make_nvp("model_path_", model_path_));
    }
};

CEREAL_CLASS_VERSION(ComponentItemSpawner, 1);
