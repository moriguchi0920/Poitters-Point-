#pragma once

#include <System/Scene.h>

namespace Sample::GameSample {

USING_PTR(Enemy);

//! @brief オブジェクト(obj)に近い敵を返す
//! @param obj
//! @return 近い敵
ObjectPtr TrackingNearEnemy(ObjectPtr obj);

//! @brief 敵
//! @detail Draw()はありません。Object標準にて処理されます
class Enemy : public Object
{
public:
    BP_OBJECT_DECL(Enemy, u8"Enemy");

    static EnemyPtr Create(const float3& pos, const float3& front = {0, 0, 1});

    //! @name システムオーバーライド系
    //! @{

    //! @brief 初期化
    //! @return 初期化できたかどうか
    bool Init() override;

    //! @brief 処理更新
    //! @param delta フレーム時間
    void Update() override;

    //! @brief GUI処理
    void GUI() override;

    //! @brief 当たりのコールバック
    //! @param hitInfo
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override;

    //! @}

    //! @name 内部アクセス用
    // @{

    //! @brief スピードを設定する
    //! @param s スピード
    void SetSpeed(float s);

    //! @brief スピードを取得する
    //! @return 現在のスピード
    float GetSpeed();

    //! @brief 変数として扱う関数
    //! @detail Set/Get系と比べデバックはしにくい
    //! @return 現在のスピードの変数(speed_)
    float& Speed();

    // @}

private:
    float speed_ = 1.0f;
    float rot_y_ = 0.0f;
};

}    // namespace Sample::GameSample
