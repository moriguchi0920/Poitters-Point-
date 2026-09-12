#include "ComponentItemSpawner.h"
#include "Game/Object/PoittersPoint_Rock.h"
// #include "Game/Object/PoittersPoint_Crate.h"  // モデル到着時にコメント解除
// #include "Game/Object/PoittersPoint_Slime.h"  // モデル到着時にコメント解除
#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <cmath>
#include <algorithm>
#include <random>

void ComponentItemSpawner::Init()
{
    __super::Init();

    LoadModel();

    move_speed_       = 90.0f;
    spawn_delay_time_ = 0.3f;
    max_active_items_ = 20;    //　ステージ上に同時に存在していられるアイテムの数

    spawner_points_.clear();

    std::random_device rd;
    std::mt19937       gen(rd());
    // 各マスの中心座標から少しずらしてまばらに生成している感を出す
    std::uniform_real_distribution<float> offset_dist(-8.0f, 8.0f);
    std::uniform_int_distribution<int>    type_dist(1, 2);    // 1: Crate, 2: Slime

    const int   grid_size = 4;    // スポーンは4*4
    const float start_pos = -45.0f;
    const float step      = 30.0f;    // 少し広めの間隔(スポーン同士)

    for(int z = 0; z < grid_size; ++z) {
        for(int x = 0; x < grid_size; ++x) {
            float px = start_pos + x * step + offset_dist(gen);
            float pz = start_pos + z * step + offset_dist(gen);

            SpawnerPoint point;
            point.position = float3(px, 0.0f, pz);

            if(type_dist(gen) == 1) {
                point.assigned_type = ItemType::Crate;
            }
            else {
                point.assigned_type = ItemType::Slime;
            }

            spawner_points_.push_back(point);
        }
    }

    spawn_timer_        = 0.0f;
    target_point_index_ = GetEmptyPointIndex();

    if(auto owner = GetOwner()) {
        if(target_point_index_ != -1) {
            float3 start_pos  = spawner_points_[target_point_index_].position;
            start_pos.y      += 35.0f;
            owner->SetTranslate(start_pos);
        }
    }
}

void ComponentItemSpawner::Update()
{
    __super::Update();

    auto owner = GetOwner();
    if(!owner)
        return;

    float delta_time = GetDeltaTime();

    if(spawn_timer_ > 0.0f) {
        spawn_timer_ -= delta_time;
    }

    if(target_point_index_ == -1 || !spawner_points_[target_point_index_].IsEmptyAndNeedsSpawn()) {
        target_point_index_ = GetEmptyPointIndex();
    }

    float3 current_pos = owner->GetTranslate();

    // 補充が必要なマスが無い場合は画面外に待機させておく
    if(target_point_index_ == -1) {
        float3 retreat_pos = float3(0.0f, 80.0f, -120.0f);
        float  dx          = retreat_pos.x - current_pos.x;
        float  dy          = retreat_pos.y - current_pos.y;
        float  dz          = retreat_pos.z - current_pos.z;
        float  dist        = std::sqrt(dx * dx + dy * dy + dz * dz);

        if(dist > 2.0f) {
            float speed    = move_speed_ * 1.5f;
            current_pos.x += (dx / dist) * speed * delta_time;
            current_pos.y += (dy / dist) * speed * delta_time;
            current_pos.z += (dz / dist) * speed * delta_time;
            owner->SetTranslate(current_pos);
        }
        else {
            float_timer_  += delta_time * 2.0f;
            current_pos.y  = retreat_pos.y + std::sin(float_timer_) * 0.01f;
            owner->SetTranslate(current_pos);
        }
        return;
    }

    float3      target_pos     = spawner_points_[target_point_index_].position;
    const float flight_height  = 35.0f;
    target_pos.y              += flight_height;

    float dx          = target_pos.x - current_pos.x;
    float dz          = target_pos.z - current_pos.z;
    float distance_xz = std::sqrt(dx * dx + dz * dz);

    if(distance_xz > 1.5f) {
        float dy       = target_pos.y - current_pos.y;
        current_pos.x += (dx / distance_xz) * move_speed_ * delta_time;
        current_pos.y += (dy / distance_xz) * move_speed_ * delta_time;
        current_pos.z += (dz / distance_xz) * move_speed_ * delta_time;
        float_timer_  += delta_time * 4.0f;
        current_pos.y += std::sin(float_timer_) * 0.015f;
        owner->SetTranslate(current_pos);
    }
    else {
        if(spawn_timer_ <= 0.0f) {
            SpawnItemAt(target_point_index_);
            spawn_timer_        = spawn_delay_time_;
            target_point_index_ = -1;
        }
        else {
            float_timer_  += delta_time * 3.0f;
            current_pos.y  = target_pos.y + std::sin(float_timer_) * 0.02f;
            owner->SetTranslate(current_pos);
        }
    }
}

int ComponentItemSpawner::GetEmptyPointIndex()
{
    size_t           current_active_count = 0;
    std::vector<int> empty_indices;

    for(size_t i = 0; i < spawner_points_.size(); ++i) {
        // まだ場に存在しているアイテムをカウント
        if(!spawner_points_[i].current_item_ptr.expired()) {
            current_active_count++;
        }
        // 空いているマスを見つけます
        else if(spawner_points_[i].IsEmptyAndNeedsSpawn()) {
            empty_indices.push_back(static_cast<int>(i));
        }
    }

    // ステージ上のアイテムが上限に達していたら補充しない
    if(current_active_count >= max_active_items_ || empty_indices.empty()) {
        return -1;
    }

    // 空きのスポーン位置の中からランダムに1つ選んでドラゴンに向かわせる
    std::random_device rd;
    std::mt19937       g(rd());
    std::shuffle(empty_indices.begin(), empty_indices.end(), g);

    return empty_indices[0];
}

ObjectPtr ComponentItemSpawner::CreateItemInstance(ItemType type)
{
    switch(type) {
    /* クラス完成時にコメント解除
    case ItemType::Crate: return Scene::Object::Create<PoittersPoint::Crate>();
    case ItemType::Slime: return Scene::Object::Create<PoittersPoint::Slime>();
    */
    case ItemType::Crate:
    case ItemType::Slime:
    default:
        return Scene::Object::Create<PoittersPoint::Rock>();
    }
}

void ComponentItemSpawner::SpawnItemAt(int point_index)
{
    if(point_index < 0 || point_index >= static_cast<int>(spawner_points_.size()))
        return;

    if(!spawner_points_[point_index].IsEmptyAndNeedsSpawn())
        return;

    float3 spawn_pos  = spawner_points_[point_index].position;
    spawn_pos.y      += 34.0f;

    ItemType type_to_spawn = spawner_points_[point_index].assigned_type;
    auto     item          = CreateItemInstance(type_to_spawn);

    if(item) {
        item->SetTranslate(spawn_pos);
        spawner_points_[point_index].current_item_ptr = item;
    }
}

void ComponentItemSpawner::GUI()
{
}

void ComponentItemSpawner::LoadModel()
{
    auto owner = GetOwner();
    if(!owner)
        return;

    if(model_path_.empty()) {
        model_path_ = "data/Game/Models/ItemSpawner/Dragon.mv1";
    }

    auto modelComp = owner->GetComponent<ComponentModel>();
    if(!modelComp) {
        modelComp = owner->AddComponent<ComponentModel>();
    }

    if(modelComp) {
        modelComp->UseShader(false);
        modelComp->Load(model_path_);
    }
}

CEREAL_REGISTER_TYPE(ComponentItemSpawner)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentItemSpawner)
