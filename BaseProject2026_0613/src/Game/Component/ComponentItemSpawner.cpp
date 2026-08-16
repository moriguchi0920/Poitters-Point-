#include "ComponentItemSpawner.h"
#include "Game/Object/PoittersPoint_Rock.h"
#include "Game/Component/ComponentGrabbable.h"
#include <System/Scene.h>
#include <cmath>

void ComponentItemSpawner::Init()
{
    __super::Init();

    LoadModel();

    move_speed_       = 70.0f;
    spawn_delay_time_ = 1.0f;

    // スポナー位置の間隔
    spawner_points_.clear();
    spawner_points_.push_back({float3(25.0f, 0.0f, 25.0f), {}});
    spawner_points_.push_back({float3(-25.0f, 0.0f, 25.0f), {}});
    spawner_points_.push_back({float3(25.0f, 0.0f, -25.0f), {}});
    spawner_points_.push_back({float3(-25.0f, 0.0f, -25.0f), {}});
    spawner_points_.push_back({float3(0.0f, 0.0f, 0.0f), {}});

    spawn_timer_        = 0.0f;
    target_point_index_ = FindNextEmptyPointIndex();

    // ドラゴンの初期位置を上空（高度 35.0f）へ配置
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

    // 各ポイントの岩の状態チェック
    for(auto& point : spawner_points_) {
        if(!point.current_rock_ptr.expired()) {
            if(auto item = point.current_rock_ptr.lock()) {
                if(auto grabbable = item->GetComponent<ComponentGrabbable>()) {
                    if(grabbable->GetIsGrabbed() && grabbable->GetOwner() == item.get()) {
                        point.current_rock_ptr.reset();
                    }
                }
            }
        }
    }

    // ターゲット地点の更新
    if(target_point_index_ == -1 || !spawner_points_[target_point_index_].IsEmpty()) {
        target_point_index_ = FindNextEmptyPointIndex();
    }

    float3 current_pos = owner->GetTranslate();

    // -------------------------------------------------------------
    //岩が全箇所に満ちている場合に画面外邪魔にならない遥か遠くへ退避
    // -------------------------------------------------------------
    if(target_point_index_ == -1) {
        // ★画面外の退避目標座標（遥か上空・遠方）
        float3 retreat_pos = float3(0.0f, 80.0f, -120.0f);

        float dx   = retreat_pos.x - current_pos.x;
        float dy   = retreat_pos.y - current_pos.y;
        float dz   = retreat_pos.z - current_pos.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        // 遠くの待機場所へ移動（少し速めのスピードで離脱）
        if(dist > 2.0f) {
            float speed    = move_speed_ * 1.5f;
            current_pos.x += (dx / dist) * speed * delta_time;
            current_pos.y += (dy / dist) * speed * delta_time;
            current_pos.z += (dz / dist) * speed * delta_time;
            owner->SetTranslate(current_pos);
        }
        else {
            // 到着したら画面外でゆったり浮遊待機
            float_timer_  += delta_time * 2.0f;
            current_pos.y  = retreat_pos.y + std::sin(float_timer_) * 0.01f;
            owner->SetTranslate(current_pos);
        }
        return;
    }

    // -------------------------------------------------------------
    // 岩が不足している場合は画面外（または現在地）から目標のスポナーへ
    // -------------------------------------------------------------
    float3      target_pos     = spawner_points_[target_point_index_].position;
    const float flight_height  = 35.0f;    // ドラゴンの作業高度
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
        // 到着後、即座（タイマー経過後）に産み落とす
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

int ComponentItemSpawner::FindNextEmptyPointIndex()
{
    for(size_t i = 0; i < spawner_points_.size(); ++i) {
        if(spawner_points_[i].IsEmpty()) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void ComponentItemSpawner::SpawnItemAt(int point_index)
{
    if(point_index < 0 || point_index >= static_cast<int>(spawner_points_.size()))
        return;

    if(!spawner_points_[point_index].IsEmpty())
        return;

    // ドラゴンの直下高空（高度 34.0f）から産み落とす
    float3 spawn_pos  = spawner_points_[point_index].position;
    spawn_pos.y      += 34.0f;

    auto rock = Scene::Object::Create<PoittersPoint::Rock>();
    if(rock) {
        rock->SetTranslate(spawn_pos);
        spawner_points_[point_index].current_rock_ptr = rock;
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
