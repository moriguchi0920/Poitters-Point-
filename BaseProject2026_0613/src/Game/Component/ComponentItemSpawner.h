#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentItemSpawner.h
//! @brief  アイテムスポナー(ドラゴン)コンポーネント
//! @details ステージ上の 4x4 のマス(スポーン位置)を巡回し、空いているマスの上まで飛んで
//!          投げ物(岩・スライム・丸太)を落とす。
//!          マスは「置いたアイテムが消えた / 誰かに持たれた・投げられた / 置き場所から離れた」時に空きになる。
//!          場に出ている投げ物の総数が上限に達している間は補充しない。
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <string>
#include <vector>
#include <random>

USING_PTR(ComponentItemSpawner);

enum class ItemType
{
    None,       //!< 何も置かないマス
    Crate,      //!< 木箱(モデル到着までは岩で代用)
    Slime,      //!< スライム
    Missile,    //!< ミサイル(未実装)
    Log,        //!< 丸太
};

//! スポーン位置(マス)
struct SpawnerPoint
{
    float3        position{0.0f, 0.0f, 0.0f};    //!< 地面の位置
    ObjectWeakPtr current_item_ptr;               //!< このマスに置いたアイテム
    ItemType      assigned_type = ItemType::Crate;
};

class ComponentItemSpawner : public Component
{
public:
    BP_COMPONENT_DECL(ComponentItemSpawner, u8"アイテムスポナーコンポーネント");

    void Init() override;
    void Update() override;
    void GUI() override;

    //! @brief 指定のマスにアイテムを置く
    void SpawnItemAt(int point_index);

    //! @brief ドラゴンのモデルを読み込む
    void LoadModel();

private:
    //! @brief 補充が必要なマスをランダムに1つ選ぶ(無ければ -1)
    int GetEmptyPointIndex();

    //! @brief マスが空いていて補充が必要か
    //! @details 置いたアイテムが消えた・持たれた/投げられた・置き場所から離れた場合に空きとみなす
    bool IsPointEmpty(const SpawnerPoint& point) const;

    //! @brief 場に出ている投げ物(キャラクター以外の掴める物)の数
    size_t CountActiveItems() const;

    //! @brief 指定のXZ位置の地面の高さ(地面が見つからなければ fallback)
    float GetGroundHeight(float x, float z, float fallback) const;

    ObjectPtr CreateItemInstance(ItemType type);

private:
    std::vector<SpawnerPoint> spawner_points_;
    int                       target_point_index_ = -1;

    size_t max_active_items_ = 20;        //!< フィールド上に同時に存在できる投げ物の最大数
    float  move_speed_       = 90.0f;     //!< ドラゴンの移動速度(単位/秒)
    float  flight_height_    = 35.0f;     //!< ドラゴンが飛ぶ高さ(地面から)
    float  drop_height_      = 34.0f;     //!< アイテムを落とす高さ(地面から)
    float  leave_distance_   = 15.0f;     //!< 置いたアイテムがこれ以上離れたらマスを空きとみなす
    float  float_timer_      = 0.0f;
    float  spawn_timer_      = 0.0f;
    float  spawn_delay_time_ = 0.3f;      //!< 1つ置いてから次を置けるまでの時間(秒)

    std::mt19937 random_{std::random_device{}()};

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
