#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentItemSpawner.h
//! @brief  アイテムスポナー(ドラゴン)コンポーネント
//! @details アイテムの種類ごとにステージ上の上限数を決めておき、
//!          場にある数が上限より少ない種類があれば、ドラゴンがステージ上のランダムな位置まで飛んで行って落とす。
//!          (岩・丸太は壊れないので上限まで置いたら増えない。スライムは爆発すると減るので補充される)
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <string>
#include <vector>
#include <random>

USING_PTR(ComponentItemSpawner);

enum class ItemType
{
    None,       //!< なし
    Crate,      //!< 木箱(モデル到着までは岩で代用)
    Slime,      //!< スライム
    Missile,    //!< ミサイル(未実装)
    Log,        //!< 丸太
};

class ComponentItemSpawner : public Component
{
public:
    BP_COMPONENT_DECL(ComponentItemSpawner, u8"アイテムスポナーコンポーネント");

    void Init() override;
    void Update() override;
    void GUI() override;

    //! @brief ドラゴンのモデルを読み込む
    void LoadModel();

    //! @brief 種類ごとの上限数を設定する
    void SetMaxCount(ItemType type, int max_count);

private:
    //! 種類ごとの設定
    struct ItemRule
    {
        ItemType    type      = ItemType::None;
        const char* name      = "";
        int         max_count = 0;    //!< ステージ上の上限数
    };

    //! @brief 場にある指定の種類のアイテムの数(持たれている・投げられている物も含む)
    int CountItems(ItemType type) const;

    //! @brief 上限より少ない種類を選ぶ(足りない数が多い種類を優先。無ければ None)
    ItemType ChooseShortageType();

    //! @brief アイテムを落とす位置をステージ上からランダムに選ぶ
    //! @retval false 適当な位置が見つからなかった
    bool ChooseDropPosition(float3& out_ground_pos);

    //! @brief 指定の位置へ飛ぶ(向きも進行方向へ回す)
    //! @retval true 到着した
    bool FlyTo(const float3& target, float speed);

    //! @brief 進行方向へモデルを向ける
    void FaceTo(const float3& move_dir);

    ObjectPtr CreateItemInstance(ItemType type);

    //! @brief アイテムを落とす
    void DropItem(ItemType type, const float3& ground_pos);

private:
    //! 種類ごとのステージ上の上限数(GUI / SetMaxCount で変更できる)
    std::vector<ItemRule> rules_ = {
        {ItemType::Crate, u8"岩(木箱の代わり)", 4},
        {  ItemType::Log,               u8"丸太", 2},
        {ItemType::Slime,           u8"スライム", 3},
    };

    //! 今向かっている落下位置
    bool     has_target_      = false;
    ItemType target_type_     = ItemType::None;
    float3   target_ground_   = {0.0f, 0.0f, 0.0f};

    float move_speed_       = 90.0f;     //!< ドラゴンの移動速度(単位/秒)
    float turn_speed_       = 6.0f;      //!< 向きを変える速さ(1/60秒あたりの最大角度)
    float model_front_rot_  = 0.0f;      //!< モデルの正面方向の補正(度)
    float flight_height_    = 35.0f;     //!< ドラゴンが飛ぶ高さ(地面から)
    float drop_height_      = 34.0f;     //!< アイテムを落とす高さ(地面から)
    float drop_wait_time_   = 0.3f;      //!< 到着してから落とすまでの時間(秒)
    float spawn_interval_   = 2.0f;      //!< 1つ落としてから次に落としに行くまでの時間(秒)
    float edge_margin_      = 30.0f;     //!< ステージの端(壁)からこれ以上内側に落とす
    float min_spacing_      = 20.0f;     //!< キャラクターや他のアイテムからこれ以上離れた所に落とす
    float height_tolerance_ = 15.0f;     //!< ステージ中央の地面からこの高さ以内の場所にだけ落とす(段差の上を避ける)

    float wait_timer_     = 0.0f;
    float interval_timer_ = 0.0f;
    float float_timer_    = 0.0f;

    std::mt19937 random_{std::random_device{}()};

    std::string model_path_ = "data/Game/Models/ItemSpawner/Dragon.mv1";

    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
        arc(cereal::make_nvp("spawn_delay_time_", drop_wait_time_));
        arc(cereal::make_nvp("move_speed_", move_speed_));
        arc(cereal::make_nvp("model_path_", model_path_));
    }
};

CEREAL_CLASS_VERSION(ComponentItemSpawner, 1);
