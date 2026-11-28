//---------------------------------------------------------------------------
//! @file   ComponentItemSpawner.cpp
//! @brief  アイテムスポナー(ドラゴン)コンポーネント
//---------------------------------------------------------------------------
#include "ComponentItemSpawner.h"
#include "Game/Object/PoittersPoint_Rock.h"
// #include "Game/Object/PoittersPoint_Crate.h"  // モデル到着時にコメント解除
#include "Game/Object/PoittersPoint_Slime.h"
#include "Game/Object/PoittersPoint_Log.h"
#include "Game/Object/PoittersPoint_Player.h"
#include "Game/Object/PoittersPoint_Ground.h"
#include "Game/Component/ComponentGrabbable.h"
#include "Game/System/PoittersPoint_PhysicsHelper.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <cmath>
#include <algorithm>

void ComponentItemSpawner::Init()
{
    __super::Init();

    LoadModel();

    spawner_points_.clear();

    // 各マスの中心座標から少しずらしてまばらに生成している感を出す
    std::uniform_real_distribution<float> offset_dist(-8.0f, 8.0f);
    std::uniform_int_distribution<int>    type_dist(1, 3);    // 1: Crate(岩), 2: Slime, 3: Log

    const int   grid_size = 4;         // スポーンは4*4
    const float start_pos = -45.0f;
    const float step      = 30.0f;     // 少し広めの間隔(スポーン同士)

    for(int z = 0; z < grid_size; ++z) {
        for(int x = 0; x < grid_size; ++x) {
            float px = start_pos + x * step + offset_dist(random_);
            float pz = start_pos + z * step + offset_dist(random_);

            SpawnerPoint point;
            // ステージの地面は平らではないため、物理のレイキャストで地面の高さを調べる
            point.position = float3(px, GetGroundHeight(px, pz, 0.0f), pz);

            // ※以前は if(type_dist(gen) == 1) と2回目の抽選をしていたため、種類の出方が偏っていた
            int type = type_dist(random_);
            if(type == 1) {
                point.assigned_type = ItemType::Crate;
            }
            else if(type == 2) {
                point.assigned_type = ItemType::Slime;
            }
            else {
                point.assigned_type = ItemType::Log;
            }
            spawner_points_.push_back(point);
        }
    }

    spawn_timer_        = 0.0f;
    target_point_index_ = GetEmptyPointIndex();

    if(auto owner = GetOwner()) {
        if(target_point_index_ != -1) {
            float3 first_pos  = spawner_points_[target_point_index_].position;
            first_pos.y      += flight_height_;
            owner->SetTranslate(first_pos);
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

    if(target_point_index_ == -1 || !IsPointEmpty(spawner_points_[target_point_index_]) || max_active_items_ <= CountActiveItems()) {
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

    float3 target_pos  = spawner_points_[target_point_index_].position;
    target_pos.y      += flight_height_;

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

bool ComponentItemSpawner::IsPointEmpty(const SpawnerPoint& point) const
{
    if(point.assigned_type == ItemType::None)
        return false;

    auto item = point.current_item_ptr.lock();
    if(!item)
        return true;    // 消えた(スライムの爆発など)

    // 岩・丸太は壊れずに残るため、「持たれた・投げられた」「置き場所から離れた」時点で
    // そのマスは使い終わったとみなして補充する(総数は max_active_items_ で抑える)
    if(auto grabbable = item->GetComponent<ComponentGrabbable>()) {
        if(grabbable->GetState() != ComponentGrabbable::State::Free)
            return true;
    }
    float3 diff = item->GetTranslate() - point.position;
    diff.y      = 0.0f;
    return leave_distance_ < (float)length(diff);
}

size_t ComponentItemSpawner::CountActiveItems() const
{
    size_t count = 0;
    for(auto& obj : Scene::Object::GetArray<Object>()) {
        // キャラクターも掴めるが、投げ物の数には含めない
        if(std::dynamic_pointer_cast<PoittersPoint::Player>(obj))
            continue;
        if(obj->GetComponent<ComponentGrabbable>())
            count++;
    }
    return count;
}

float ComponentItemSpawner::GetGroundHeight(float x, float z, float fallback) const
{
    if(auto ground = Scene::Object::Get<PoittersPoint::Ground>()) {
        float y = 0.0f;
        if(ground->FindGroundHeight(x, z, y))
            return y;
    }
    return fallback;
}

int ComponentItemSpawner::GetEmptyPointIndex()
{
    // ステージ上の投げ物が上限に達していたら補充しない
    if(max_active_items_ <= CountActiveItems()) {
        return -1;
    }

    std::vector<int> empty_indices;
    for(size_t i = 0; i < spawner_points_.size(); ++i) {
        if(IsPointEmpty(spawner_points_[i])) {
            empty_indices.push_back(static_cast<int>(i));
        }
    }
    if(empty_indices.empty()) {
        return -1;
    }

    // 空きのスポーン位置の中からランダムに1つ選んでドラゴンに向かわせる
    std::uniform_int_distribution<size_t> pick(0, empty_indices.size() - 1);
    return empty_indices[pick(random_)];
}

ObjectPtr ComponentItemSpawner::CreateItemInstance(ItemType type)
{
    switch(type) {
    /* クラス完成時にコメント解除
    case ItemType::Crate: return Scene::Object::Create<PoittersPoint::Crate>();
    */
    case ItemType::Crate:
        return Scene::Object::Create<PoittersPoint::Rock>();
    case ItemType::Slime:
        return Scene::Object::Create<PoittersPoint::Slime>();
    case ItemType::Log:
        return Scene::Object::Create<PoittersPoint::Log>();
    default:
        return Scene::Object::Create<PoittersPoint::Rock>();
    }
}

void ComponentItemSpawner::SpawnItemAt(int point_index)
{
    if(point_index < 0 || point_index >= static_cast<int>(spawner_points_.size()))
        return;

    auto& point = spawner_points_[point_index];
    if(!IsPointEmpty(point))
        return;

    float3 spawn_pos  = point.position;
    spawn_pos.y      += drop_height_;

    auto item = CreateItemInstance(point.assigned_type);
    if(item) {
        // 投げ物は物理ボディを持つため、SetTranslate ではなく Warp で物理ボディごと移動する
        // (SetTranslate だけだと次の物理更新で元の位置に戻されてしまう)
        PoittersPoint::PhysicsHelper::Warp(item.get(), spawn_pos);
        point.current_item_ptr = item;
    }
}

void ComponentItemSpawner::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode(u8"Item Spawner")) {
            int empty_count = 0;
            for(auto& point : spawner_points_) {
                if(IsPointEmpty(point))
                    empty_count++;
            }
            ImGui::Text(u8"場の投げ物: %d / %d  空きマス: %d / %d",
                        static_cast<int>(CountActiveItems()),
                        static_cast<int>(max_active_items_),
                        empty_count,
                        static_cast<int>(spawner_points_.size()));
            ImGui::Text(u8"向かっているマス: %d", target_point_index_);

            int max_items = static_cast<int>(max_active_items_);
            if(ImGui::DragInt(u8"投げ物の最大数", &max_items, 1, 0, 100))
                max_active_items_ = static_cast<size_t>(std::max(0, max_items));
            ImGui::DragFloat(u8"移動速度", &move_speed_, 1.0f, 1.0f, 500.0f);
            ImGui::DragFloat(u8"次に置くまでの時間(秒)", &spawn_delay_time_, 0.05f, 0.0f, 30.0f);
            ImGui::DragFloat(u8"置き場所から離れたら空きとみなす距離", &leave_distance_, 0.5f, 1.0f, 200.0f);
            ImGui::TreePop();
        }
    }
    ImGui::End();
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
