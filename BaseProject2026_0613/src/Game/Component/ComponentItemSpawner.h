#pragma once

#include <System/Scene.h>
#include <System/Component/Component.h>
#include <string>
#include <vector>

USING_PTR(ComponentItemSpawner);

//! @brief スポナー地点の情報構造体
struct SpawnerPoint
{
    float3        position{0.0f, 0.0f, 0.0f};    // 地点の座標
    ObjectWeakPtr current_rock_ptr;              // 現在配置されているRockの参照

    //! @brief その場所にRockが無い（持ち上げられた/破棄された）か判定
    bool IsEmpty() const { return current_rock_ptr.expired(); }
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
    //! @brief 岩がない（IsEmpty）スポナー地点を検索する
    int FindNextEmptyPointIndex();

private:
    // スポナー地点（5箇所）の管理
    std::vector<SpawnerPoint> spawner_points_;
    int                       target_point_index_ = -1;    // ドラゴンが現在向かっている目標地点のインデックス

    // ドラゴンのパラメータ
    float move_speed_  = 5.0f;    // 移動速度
    float float_timer_ = 0.0f;    // 上下タイマー

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
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
        arc(cereal::make_nvp("spawn_delay_time_", spawn_delay_time_));
        arc(cereal::make_nvp("move_speed_", move_speed_));
        arc(cereal::make_nvp("model_path_", model_path_));
    }
    //@}
};

CEREAL_CLASS_VERSION(ComponentItemSpawner, 1);
