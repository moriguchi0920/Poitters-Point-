#include "ComponentItemSpawner.h"
#include "Game/Object/PoittersPoint_Rock.h"
#include "Game/Component/ComponentGrabbable.h"
#include <System/Scene.h>
#include <cmath>
#include <algorithm>

void ComponentItemSpawner::Init()
{
    __super::Init();

    // 乱数エンジンの初期化
    std::random_device rd;
    random_engine_.seed(rd());

    LoadModel();

    move_speed_       = 90.0f;
    spawn_delay_time_ = 0.3f;

    // -------------------------------------------------------------
    // スポナー候補地の範囲調整（画面内にきれいに収まるサイズ感）
    // -------------------------------------------------------------
    spawner_points_.clear();

    const int   grid_size = 5;         // 5x5 = 25箇所のグリッド
    const float start_pos = -60.0f;    // 開始位置をやや内側に絞る（旧: -100.0f）
    const float step      = 30.0f;     // 間隔を縮小（旧: 40.0f）
    // これにより -60.0, -30.0, 0.0, 30.0, 60.0 の範囲に生成されます

    for(int z = 0; z < grid_size; ++z) {
        for(int x = 0; x < grid_size; ++x) {
            float px = start_pos + x * step;
            float pz = start_pos + z * step;
            spawner_points_.push_back({float3(px, 0.0f, pz), {}});
        }
    }

    spawn_timer_        = 0.0f;
    target_point_index_ = GetRandomEmptyPointIndex();

    // ドラゴンの初期位置
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

    // 各ポイントのアイテムの状態をチェック（掴まれたら解放）
    for(auto& point : spawner_points_) {
        if(!point.current_item_ptr.expired()) {
            if(auto item = point.current_item_ptr.lock()) {
                if(auto grabbable = item->GetComponent<ComponentGrabbable>()) {
                    if(grabbable->GetIsGrabbed() && grabbable->GetOwner() == item.get()) {
                        point.current_item_ptr.reset();
                    }
                }
            }
        }
    }

    // ターゲット地点の更新（未設定または埋まったら再選択）
    if(target_point_index_ == -1 || !spawner_points_[target_point_index_].IsEmpty()) {
        target_point_index_ = GetRandomEmptyPointIndex();
    }

    float3 current_pos = owner->GetTranslate();

    // -------------------------------------------------------------
    // 目標地点がない場合（または全17個埋まっている場合）、上空退避
    // -------------------------------------------------------------
    if(target_point_index_ == -1) {
        float3 retreat_pos = float3(0.0f, 80.0f, -120.0f);

        float dx   = retreat_pos.x - current_pos.x;
        float dy   = retreat_pos.y - current_pos.y;
        float dz   = retreat_pos.z - current_pos.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

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

    // ------------------------------------------------------------------------
    // 目標のスポーン地点へ移動・配置
    // ------------------------------------------------------------------------
    float3      target_pos     = spawner_points_[target_point_index_].position;
    const float flight_height  = 35.0f;
    target_pos.y              += flight_height;

    float dx          = target_pos.x - current_pos.x;
    float dz          = target_pos.z - current_pos.z;
    float distance_xz = std::sqrt(dx * dx + dz * dz);

    if(distance_xz > 1.5f) {
        float dy = target_pos.y - current_pos.y;

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

void ComponentItemSpawner::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("Item Spawner (Dragon)")) {
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable)) {
                SetStatus(StatusBit::Enable, enable);
            }

            char buffer[256];
            strncpy_s(buffer, model_path_.c_str(), sizeof(buffer));
            if(ImGui::InputText(u8"モデルパス", buffer, sizeof(buffer))) {
                model_path_ = buffer;
                LoadModel();
            }

            ImGui::DragFloat(u8"移動速度", &move_speed_, 0.5f, 0.0f, 100.0f);
            ImGui::DragFloat(u8"再生成時間(秒)", &spawn_delay_time_, 0.1f, 0.0f, 60.0f);

            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

int ComponentItemSpawner::GetRandomEmptyPointIndex()
{
    constexpr int   MAX_FIELD_ITEMS = 17;       // フィールド上の最大個数
    constexpr float MIN_DISTANCE    = 25.0f;    // グリッドを狭めたため最小距離も縮小（旧: 35.0f）

    // 1. 現在のフィールド上の総アイテム数をカウント
    int current_item_count = 0;
    for(const auto& point : spawner_points_) {
        if(!point.IsEmpty()) {
            current_item_count++;
        }
    }

    // 2. 17個以上なら生成をストップ
    if(current_item_count >= MAX_FIELD_ITEMS) {
        return -1;
    }

    // 3. 空いているポイントを収集
    std::vector<int> empty_indices;
    for(size_t i = 0; i < spawner_points_.size(); ++i) {
        if(spawner_points_[i].IsEmpty()) {
            empty_indices.push_back(static_cast<int>(i));
        }
    }

    if(empty_indices.empty()) {
        return -1;
    }

    // ランダム順にシャッフル
    std::shuffle(empty_indices.begin(), empty_indices.end(), random_engine_);

    // 4. すでに存在する岩と離れているポイント（MIN_DISTANCE以上）を最優先で選ぶ
    for(int idx : empty_indices) {
        float3 candidate_pos = spawner_points_[idx].position;
        bool   too_close     = false;

        for(const auto& point : spawner_points_) {
            if(!point.IsEmpty()) {
                float dx   = candidate_pos.x - point.position.x;
                float dz   = candidate_pos.z - point.position.z;
                float dist = std::sqrt(dx * dx + dz * dz);

                if(dist < MIN_DISTANCE) {
                    too_close = true;
                    break;
                }
            }
        }

        if(!too_close) {
            return idx;
        }
    }

    // 離れている場所がなければ、通常の空き場所から選択
    return empty_indices[0];
}

ItemType ComponentItemSpawner::DecideNextItemType()
{
    return ItemType::Rock;
}

ObjectPtr ComponentItemSpawner::CreateItemInstance(ItemType type)
{
    switch(type) {
    case ItemType::Rock:
    default:
        return Scene::Object::Create<PoittersPoint::Rock>();
    }
}

void ComponentItemSpawner::SpawnItemAt(int point_index)
{
    if(point_index < 0 || point_index >= static_cast<int>(spawner_points_.size()))
        return;

    if(!spawner_points_[point_index].IsEmpty())
        return;

    float3 spawn_pos  = spawner_points_[point_index].position;
    spawn_pos.y      += 34.0f;

    ItemType next_type = DecideNextItemType();
    auto     item      = CreateItemInstance(next_type);

    if(item) {
        item->SetTranslate(spawn_pos);
        spawner_points_[point_index].current_item_ptr = item;
    }
}

void ComponentItemSpawner::LoadModel()
{
    auto owner = GetOwner();
    if(!owner)
        return;
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
