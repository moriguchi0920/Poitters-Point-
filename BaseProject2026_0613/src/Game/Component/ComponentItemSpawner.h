#pragma once

#include <System/Scene.h>
#include <System/Component/Component.h>
#include <string>
#include <vector>

USING_PTR(ComponentItemSpawner);

enum class ItemType
{
    None,       // 壊れないアイテム
    Crate,      // 木箱
    Slime,      // スライム
    Missile,    // ミサイル
};

struct SpawnerPoint
{
    float3        position{0.0f, 0.0f, 0.0f};
    ObjectWeakPtr current_item_ptr;
    ItemType      assigned_type = ItemType::Crate;

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
    int       GetEmptyPointIndex();
    ObjectPtr CreateItemInstance(ItemType type);

private:
    std::vector<SpawnerPoint> spawner_points_;
    int                       target_point_index_ = -1;

    // フィールド上に同時に存在できる最大アイテム数
    size_t max_active_items_ = 20;

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
