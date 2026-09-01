#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Character.h
//! @brief  PoittersPoint_Character
//---------------------------------------------------------------------------
#include <System/Scene.h>

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

USING_PTR(Character);
// キャラクター基底クラス
// プレイヤーとNPCはこのクラスをインスタンス生成、データを入力、ステートマシンクラスを足すことで表現する
class Character : public Object
{
public:
    BP_OBJECT_DECL(Character, "PoittersPoint::Character");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

    void SetCharacterStatus(const CharacterData& data);

    float GetMoveSpeed();

private:
    // モデルのファイルパス
    std::string model_path_;

    // アニメーションの名前、ファイルパス、アニメーションのファイル内番号、再生速度のセットの配列(BP)
    // {設定したい名前、ファイルパス、アニメーション番号、再生速度}で入力する
    // あとはモデルのSetAnimation関数の引数に入れる形で組み込める
    std::vector<Animation::Desc> anims_;

    // モデルのサイズが合わない場合のスケール行列設定用
    float3 local_scale_;
    // 移動スピード
    float move_speed_;
    // 投げたオブジェクトがどのくらいの倍率で飛距離が増すか(1.0を基準？必要なければ削除予定)
    float throw_power_;
    // オブジェクトを持ち上げる時にかかる時間に対してかける倍率(1.0でそのオブジェクトのデフォルトの時間がかかるとする)
    // 力が強い設定のキャラクターの場合値が小さくなる
    float lift_time_ratio_;
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Character);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Character);
