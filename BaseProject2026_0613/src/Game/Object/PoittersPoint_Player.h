#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Player.h
//! @brief  PoittersPoint_Player
//---------------------------------------------------------------------------
#include <System/Scene.h>

class ComponentGrabbable;
namespace PoittersPoint {
// namespace PoittersPoint

struct CharacterData
{
    std::string                  model_path_;
    std::vector<Animation::Desc> anims_;
    float3                       local_scale_;
    float                        move_speed_;
    float                        throw_power_;
    float                        lift_time_ratio_;
    CharacterData(std::string model_path, std::vector<Animation::Desc> anims, float3 local_scale, float move_speed, float throw_power, float lift_time_ratio)
        : model_path_(model_path)
        , anims_(anims)
        , local_scale_(local_scale)
        , move_speed_(move_speed)
        , throw_power_(throw_power)
        , lift_time_ratio_(lift_time_ratio)
    {
    }
};

USING_PTR(Player);
class Player : public Object
{
public:
    BP_OBJECT_DECL(Player, "PoittersPoint::Player");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief 更新
    //! 視界内の掴み可能対象を検知する
    void Update() override;

    //! @brief 当たり判定のコールバック
    //! 投げられた物体が移動中に当たった場合、ダメージを与える
    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

    //! @brief 視界判定
    //! 正面45度・距離50以内の掴み可能対象を検知する(プレイヤーとCPU共通)
    void OnEyeSight();

    //! @brief 視線判定で捕らえた一番近い掴み可能対象を取得する
    const ObjectWeakPtr& GetEyeSightTarget() const;

    //! @brief ステータスデータを入力する(プレイヤーとCPUの両方で使用する)
    void SetCharacterStatus(const CharacterData& data);

    //! @brief 移動スピードを取得する
    float GetMoveSpeed();

private:
    // 視線判定で捕らえた一番近い掴み可能対象
    ObjectWeakPtr eyesight_target_;

    // モデルのファイルパス
    std::string model_path_ = "data/Game/Models/Player/Player.mv1";

    // アニメーションの名前、ファイルパス、アニメーションのファイル内番号、再生速度のセットの配列(BP)
    // {設定したい名前、ファイルパス、アニメーション番号、再生速度}で入力する
    // あとはモデルのSetAnimation関数の引数に入れる形で組み込める
    std::vector<Animation::Desc> anims_;

    // モデルのサイズが合わない場合のスケール行列設定用
    float3 local_scale_ = {0.11f, 0.11f, 0.11f};
    // 移動スピード
    float move_speed_ = 0.3f;
    // 投げたオブジェクトがどのくらいの倍率で飛距離が増すか(1.0を基準？必要なければ削除予定)
    float throw_power_ = 1.0f;
    // オブジェクトを持ち上げる時にかかる時間に対してかける倍率(1.0でそのオブジェクトのデフォルトの時間がかかるとする)
    // 力が強い設定のキャラクターの場合値が小さくなる
    float lift_time_ratio_ = 1.0f;
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Player);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Player);
