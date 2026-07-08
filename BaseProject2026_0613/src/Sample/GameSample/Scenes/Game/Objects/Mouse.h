#pragma once

#include <System/Scene.h>
#include "../Gauge.h"

namespace Sample::GameSample {
USING_PTR(Mouse);

//! @brief プレイヤー Mouse
//! @detail Draw()は存在しません。Object標準にて描画されます
class Mouse : public Object
{
public:
    BP_OBJECT_DECL(Mouse, "GameSample/Mouse");

    //! @brief 生成関数
    static MousePtr Create(const float3& pos, const float3& front = {0, 0, 1});

    //! @name システムオーバーライド系
    // @{

    bool Init() override;

    void Update() override;

    // 基本描画の後に処理します
    void LateDraw() override;

    void GUI() override;

    void OnHit(const ComponentCollision::HitInfo& hitInfo) override;

    // @}
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
    float rot_x_ = 0.0f;

    HP::Gauge gauge_;
};

}    // namespace Sample::GameSample
