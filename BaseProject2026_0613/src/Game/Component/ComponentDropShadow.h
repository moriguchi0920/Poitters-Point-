#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentDropShadow.h
//! @brief  物の真下に丸い影を描くコンポーネント
//! @details 物やキャラクターの真下(フィールドの床)に常に丸い影を描く。
//!          高いほど影は小さく薄くなるので、投げられて飛んでいる物やジャンプ中のキャラクターがどこに落ちるか分かる。
//!          ComponentGrabbable が Init で自動的に付ける(投げ物・キャラクター共通)。ドラゴンにも付けている。
//!          SetOnlyAirborne(true) にすると、空中にいる時だけ表示する。
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentDropShadow);

class ComponentDropShadow : public Component
{
public:
    BP_COMPONENT_DECL(ComponentDropShadow, u8"落下影コンポーネント");

    void Init() override;
    void Update() override;
    void Draw() override;
    void GUI() override;

    //! @brief キャラクターの影の半径を設定する。0以下なら当たり判定のカプセルの半径から自動で決める
    void SetRadius(float radius) { radius_ = radius; }

    //! @brief 物の影の大きさの倍率(モデルの形に対して)
    void SetSizeScale(float scale) { size_scale_ = scale; }

    //! @brief 空中にいる時だけ影を出すか(false: 常に出す)
    void SetOnlyAirborne(bool only_airborne) { only_airborne_ = only_airborne; }

private:
    //! @brief 今空中にいるか(影を出すべきか)
    bool IsAirborne() const;

    //! @brief 影の形の準備(キャラクターか物かを判定し、物はモデルの範囲を覚える)
    //! @retval false まだ準備できない(モデルの読み込み前など)
    bool SetupShape();

    //! @brief 今の影の形を求める(中心と、楕円の2つの半径ベクトル(XZ))
    bool CalcShadowShape(float3& center, float2& axis1, float2& axis2) const;

    bool   has_shape_    = false;                   //!< 形の準備ができたか
    bool   is_character_ = false;                   //!< キャラクター(円の影)か
    float3 mesh_center_  = {0.0f, 0.0f, 0.0f};      //!< モデル自身の座標での頂点範囲の中心
    float3 mesh_half_    = {1.0f, 1.0f, 1.0f};      //!< モデル自身の座標での頂点範囲の半分の大きさ

    bool  only_airborne_   = false;     //!< 空中にいる時だけ影を出すか(false: 常に出す)
    float radius_          = 0.0f;      //!< キャラクターの影の半径(0以下ならカプセルの半径から自動計算)
    float size_scale_      = 1.15f;     //!< 物の影の大きさの倍率(物の真下に隠れないよう少し大きめ)
    float fade_            = 0.0f;      //!< 表示の濃さ(0～1。出し入れを滑らかにする)
    float fade_speed_      = 8.0f;      //!< 出し入れの速さ(1/秒)
    float max_alpha_       = 140.0f;    //!< 一番濃い時の不透明度(0～255)
    float shrink_height_   = 120.0f;    //!< この高さで影が一番小さく薄くなる
    float min_scale_       = 0.4f;      //!< 一番高い時の影の大きさの倍率
    float floor_offset_    = 0.3f;      //!< 床とのちらつき防止のために浮かせる量

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
    //@}
};

CEREAL_CLASS_VERSION(ComponentDropShadow, 1);
