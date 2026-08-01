#include "ComponentItemSpawner.h"
#include "Game/Object/PoittersPoint_Rock.h"
#include "Game/Component/ComponentGrabbable.h"
#include <System/Scene.h>

void ComponentItemSpawner::Init()
{
    __super::Init();

    // スポナー本体のモデルを設定・読み込み
    LoadModel();

    // 起動時はタイマーなしで即座にアイテムを生成
    spawn_timer_ = 0.0f;
    SpawnItem();
}

void ComponentItemSpawner::Update()
{
    __super::Update();

    // -------------------------------------------------------------
    // 1. 監視対象の岩がまだワールドに存在している間
    // -------------------------------------------------------------
    if(!Spawn_Item_ptr_.expired()) {
        if(auto item = Spawn_Item_ptr_.lock()) {
            if(auto grabbable = item->GetComponent<ComponentGrabbable>()) {
                // 持ち上げられたオブジェクトがRockだった場合
                if(grabbable->GetIsGrabbed() && grabbable->GetOwner() == item.get()) {
                    Spawn_Item_ptr_.reset();

                    // 5秒のタイマースタート
                    spawn_timer_ = spawn_delay_time_;
                }
            }
        }
    }
    // -------------------------------------------------------------
    // 2. スポナーの上から岩が無くなった場合（持ち上げられたとき）
    // -------------------------------------------------------------
    else {
        if(spawn_timer_ > 0.0f) {
            // 5秒のカウントダウン
            spawn_timer_ -= GetDeltaTime();
        }
        else {
            // 5秒経ったら新しい岩を生成！
            SpawnItem();
        }
    }
}

void ComponentItemSpawner::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("Item Spawner")) {
            // コンポーネント有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable)) {
                SetStatus(StatusBit::Enable, enable);
            }

            // GUI上でモデルパスを入力・変更可能にする
            char buffer[256];
            strncpy_s(buffer, model_path_.c_str(), sizeof(buffer));
            if(ImGui::InputText(u8"モデルパス", buffer, sizeof(buffer))) {
                model_path_ = buffer;
                LoadModel();
            }

            // GUI上で再生成までの秒数を自由に調整できるようにする
            ImGui::DragFloat(u8"再生成時間(秒)", &spawn_delay_time_, 0.1f, 0.0f, 60.0f);

            // コンポーネント削除ボタン
            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            ImGui::Separator();

            // デバッグ表示：現在の状態とカウントダウンの状況
            if(!Spawn_Item_ptr_.expired()) {
                ImGui::Text(u8"ステータス: アイテム設置中");
            }
            else if(spawn_timer_ > 0.0f) {
                ImGui::Text(u8"ステータス: 再生成まで %.1f 秒", spawn_timer_);
            }
            else {
                ImGui::Text(u8"ステータス: スポーン準備完了");
            }

            // 強制スポーンボタン
            if(ImGui::Button(u8"即時スポーン")) {
                spawn_timer_ = 0.0f;
                SpawnItem();
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentItemSpawner::SpawnItem()
{
    auto owner = GetOwner();
    if(!owner)
        return;

    // 既にアイテムが存在している場合は二重生成しない
    if(!Spawn_Item_ptr_.expired())
        return;

    // スポナーの少し上に生成位置を設定
    float3 spawn_pos  = owner->GetTranslate();
    spawn_pos.y      += 2.0f;

    // 新しい岩を生成して位置をセット
    auto rock = Scene::Object::Create<PoittersPoint::Rock>();
    if(rock) {
        rock->SetTranslate(spawn_pos);

        Spawn_Item_ptr_ = rock;
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
