#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentGrabbable.h
//! @brief  掴み可能コンポーネント(投げ物・キャラクター共通)
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>
// std::setを使用するために必要
#include <set>

USING_PTR(ComponentGrabbable);

class ComponentGrabbable : public Component
{
public:
    BP_COMPONENT_DECL(ComponentGrabbable, u8"掴み可能コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    //! @brief リフト時間を設定
    void SetLiftTime(float time);
    //! @brief リフト時間を取得
    const float& GetLiftTime() const;
    //! @brief 移動量を設定
    void SetTranslation(float3 new_translation);
    //! @brief 移動量を加算
    void AddTranslation(float3 add_translation);
    //! @brief 移動量を取得
    const float3& GetTranslation();
    //! @brief 掴み状態を設定
    void SetIsGrabbed(bool grabbed);
    //! @brief 掴み状態を取得
    bool GetIsGrabbed();
    //! @brief 掴み可能か取得
    bool GetCanGrab();
    //! @brief 掴み可能を設定
    void SetCanGrab(const bool can_grab);
    //! @brief バウンスオフセットを設定
    void SetBounceOffset(float offset);
    //! @brief バウンスを実行
    void Bounce();
    //! @brief 接地中か判定
    bool IsGrounded();
    //! @brief 移動中か判定(XZ成分)
    bool IsMoving();
    //! @brief ダメージ値を設定(投げ物ごとに異なるダメージを持たせる)
    void SetDamage(float damage);
    //! @brief ダメージ値を取得
    float GetDamage() const;

    //! @brief Hit記録をクリア(投げ直したときに呼ぶ)
    void ClearHitTargets();
    //! @brief 既にHitしたターゲットか判定
    bool IsAlreadyHit(ObjectPtr target) const;
    //! @brief Hit記録に追加
    void AddHitTarget(ObjectPtr target);

    //! @brief 投げ主を設定
    void SetThrower(ObjectPtr thrower);
    //! @brief 投げ主か判定
    bool IsThrower(ObjectPtr target) const;

private:
    // ※初期値が無いと不定値になり、出現直後から「投げられて移動中」と判定されてしまう
    //   (スライムが出現直後に爆発する、プレイヤー同士が触れただけでダメージを受ける原因だった)
    float lift_time_     = 1.0f;     //!< リフト時間
    float bounce_offset_ = 0.0f;     //!< バウンスオフセット
    bool  is_grabbed_    = false;    //!< 掴まれているか
    bool  can_grab_this_ = true;     //!< 掴み可能か
    float damage_        = 1.0f;     //!< このオブジェクトのダメージ値

    float3 translation_ = {0.0f, 0.0f, 0.0f};    //!< 現在の移動量

    // std::setはSTLに含まれる「集合」を扱うためのデータ構造
    // 特徴は、要素が自動的にソートされ、要素の重複を許さない。(検索も早い)
    std::set<ObjectPtr> hit_targets_;    //!< 既にダメージを与えたターゲットの記録

    ObjectWeakPtr thrower_;    //!< 投げた本人（ダメージスキップ用）

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentGrabbable, 1);
