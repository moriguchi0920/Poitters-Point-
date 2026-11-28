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

    has_target_     = false;
    wait_timer_     = 0.0f;
    interval_timer_ = 0.0f;
}

void ComponentItemSpawner::SetMaxCount(ItemType type, int max_count)
{
    for(auto& rule : rules_) {
        if(rule.type == type) {
            rule.max_count = max_count;
            return;
        }
    }
}

void ComponentItemSpawner::Update()
{
    __super::Update();

    auto owner = GetOwner();
    if(!owner)
        return;

    const float dt = GetDeltaTime();
    if(interval_timer_ > 0.0f)
        interval_timer_ -= dt;

    //-----------------------------------------------------------
    // 次に落とす物と場所を決める
    //-----------------------------------------------------------
    if(!has_target_ && interval_timer_ <= 0.0f) {
        ItemType type = ChooseShortageType();
        if(type != ItemType::None && ChooseDropPosition(target_ground_)) {
            target_type_ = type;
            has_target_  = true;
            wait_timer_  = drop_wait_time_;
        }
    }

    //-----------------------------------------------------------
    // 落とす物が無い時は画面外で待機する
    //-----------------------------------------------------------
    if(!has_target_) {
        const float3 retreat_pos = float3(0.0f, 80.0f, -120.0f);
        if(FlyTo(retreat_pos, move_speed_ * 1.5f)) {
            float_timer_ += dt * 2.0f;
            float3 pos    = owner->GetTranslate();
            pos.y         = retreat_pos.y + std::sin(float_timer_) * 0.5f;
            owner->SetTranslate(pos);
        }
        return;
    }

    //-----------------------------------------------------------
    // 落とす位置の上空まで飛んで、少し待ってから落とす
    //-----------------------------------------------------------
    float3 above = target_ground_ + float3(0.0f, flight_height_, 0.0f);
    if(FlyTo(above, move_speed_)) {
        wait_timer_ -= dt;
        if(wait_timer_ <= 0.0f) {
            // 向かっている間に上限に達していたら(他の方法で増えた等)落とさない
            if(ChooseShortageType() != ItemType::None)
                DropItem(target_type_, target_ground_);
            has_target_     = false;
            interval_timer_ = spawn_interval_;
        }
    }
}

bool ComponentItemSpawner::FlyTo(const float3& target, float speed)
{
    auto   owner = GetOwner();
    float3 pos   = owner->GetTranslate();
    float3 diff  = target - pos;

    float3 diff_xz = {diff.x, 0.0f, diff.z};
    float  dist_xz = (float)length(diff_xz);
    float  dist    = (float)length(diff);
    if(dist < 1.5f)
        return true;

    // 1フレームで行き過ぎないようにする
    float step = std::min(speed * GetDeltaTime(), dist);
    pos       += diff * (step / dist);

    // 上下にふわふわ揺らす
    float_timer_ += GetDeltaTime() * 4.0f;
    pos.y        += std::sin(float_timer_) * 0.05f;
    owner->SetTranslate(pos);

    // 水平方向に動いている時だけ進行方向へ向ける(真上・真下への移動では向きを変えない)
    if(dist_xz > 0.5f)
        FaceTo(diff_xz / dist_xz);
    return false;
}

void ComponentItemSpawner::FaceTo(const float3& move_dir)
{
    auto model = GetOwner()->GetComponent<ComponentModel>();
    if(!model)
        return;

    // モデルごとの正面のずれを補正して、進行方向へ少しずつ回す
    auto rot = quaternion::rotation_axis({0, 1, 0}, model_front_rot_ * DegToRad);
    model->SetRotationToVectorWithLimit(mul(move_dir, rot), turn_speed_);
}

int ComponentItemSpawner::CountItems(ItemType type) const
{
    int count = 0;
    for(auto& obj : Scene::Object::GetArray<Object>()) {
        bool match = false;
        switch(type) {
        case ItemType::Crate:
            match = std::dynamic_pointer_cast<PoittersPoint::Rock>(obj) != nullptr;
            break;
        case ItemType::Slime:
            match = std::dynamic_pointer_cast<PoittersPoint::Slime>(obj) != nullptr;
            break;
        case ItemType::Log:
            match = std::dynamic_pointer_cast<PoittersPoint::Log>(obj) != nullptr;
            break;
        default:
            break;
        }
        if(match)
            count++;
    }
    return count;
}

ItemType ComponentItemSpawner::ChooseShortageType()
{
    // 足りない数が一番多い種類を選ぶ(同じなら候補の中からランダム)
    std::vector<ItemType> candidates;
    int                   best_shortage = 0;
    for(auto& rule : rules_) {
        int shortage = rule.max_count - CountItems(rule.type);
        if(shortage <= 0)
            continue;
        if(best_shortage < shortage) {
            best_shortage = shortage;
            candidates.clear();
        }
        if(shortage == best_shortage)
            candidates.push_back(rule.type);
    }
    if(candidates.empty())
        return ItemType::None;

    std::uniform_int_distribution<size_t> pick(0, candidates.size() - 1);
    return candidates[pick(random_)];
}

bool ComponentItemSpawner::ChooseDropPosition(float3& out_ground_pos)
{
    auto ground = Scene::Object::Get<PoittersPoint::Ground>();
    if(!ground)
        return false;

    // フィールド(外周の壁の内側)を囲む四角形の中からランダムに選び、
    // フィールドの境界から edge_margin_ 以上内側の場所だけを使う
    float min_x, max_x, min_z, max_z;
    ground->GetFieldRect(min_x, max_x, min_z, max_z);
    if(max_x <= min_x || max_z <= min_z)
        return false;

    // フィールド中央の地面の高さ(段差の上に落とさないための基準)
    float3 center   = ground->GetFieldCenter();
    float  center_y = 0.0f;
    if(!ground->FindGroundHeight(center.x, center.z, center_y))
        center_y = 0.0f;

    std::uniform_real_distribution<float> rand_x(min_x, max_x);
    std::uniform_real_distribution<float> rand_z(min_z, max_z);

    // 条件に合う場所が見つかるまで何回か試す
    for(int attempt = 0; attempt < 30; ++attempt) {
        float x = rand_x(random_);
        float z = rand_z(random_);
        if(!ground->IsInsideField(x, z, edge_margin_))
            continue;
        float y = 0.0f;
        if(!ground->FindGroundHeight(x, z, y))
            continue;
        if(height_tolerance_ < std::fabs(y - center_y))
            continue;

        // キャラクターや他のアイテム(掴める物)のすぐ近くには落とさない
        bool too_close = false;
        for(auto& obj : Scene::Object::GetArray<Object>()) {
            if(!obj->GetComponent<ComponentGrabbable>())
                continue;
            float3 d = obj->GetTranslate() - float3(x, y, z);
            d.y      = 0.0f;
            if((float)length(d) < min_spacing_) {
                too_close = true;
                break;
            }
        }
        if(too_close)
            continue;

        out_ground_pos = {x, y, z};
        return true;
    }
    return false;
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
        return nullptr;
    }
}

void ComponentItemSpawner::DropItem(ItemType type, const float3& ground_pos)
{
    auto item = CreateItemInstance(type);
    if(!item)
        return;

    // 投げ物は物理ボディを持つため、SetTranslate ではなく Warp で物理ボディごと移動する
    // (SetTranslate だけだと次の物理更新で元の位置に戻されてしまう)
    PoittersPoint::PhysicsHelper::Warp(item.get(), ground_pos + float3(0.0f, drop_height_, 0.0f));
}

void ComponentItemSpawner::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode(u8"Item Spawner")) {
            ImGui::Text(u8"種類ごとの数 (場にある数 / 上限)");
            for(auto& rule : rules_) {
                ImGui::PushID(static_cast<int>(rule.type));
                ImGui::Text("%s: %d", rule.name, CountItems(rule.type));
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100.0f);
                ImGui::DragInt(u8"上限", &rule.max_count, 0.1f, 0, 30);
                ImGui::PopID();
            }
            ImGui::Text(u8"状態: %s", has_target_ ? u8"落としに向かっている" : u8"待機中");

            ImGui::Separator();
            ImGui::DragFloat(u8"移動速度", &move_speed_, 1.0f, 1.0f, 500.0f);
            ImGui::DragFloat(u8"向きを変える速さ", &turn_speed_, 0.1f, 0.1f, 60.0f);
            ImGui::DragFloat(u8"モデルの正面の補正(度)", &model_front_rot_, 1.0f, -180.0f, 180.0f);
            ImGui::DragFloat(u8"次に落としに行くまでの時間(秒)", &spawn_interval_, 0.1f, 0.0f, 60.0f);
            ImGui::DragFloat(u8"フィールドの境界からの余白", &edge_margin_, 1.0f, 0.0f, 500.0f);
            ImGui::DragFloat(u8"キャラ・アイテムとの最小間隔", &min_spacing_, 0.5f, 0.0f, 200.0f);
            ImGui::DragFloat(u8"落とせる高さの範囲(中央の地面から)", &height_tolerance_, 0.5f, 0.0f, 500.0f);
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
