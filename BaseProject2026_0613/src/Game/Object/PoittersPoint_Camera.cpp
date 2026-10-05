//---------------------------------------------------------------------------
//! @file   PoittersPoint_Camera.cpp
//! @brief  PoittersPoint_Camera
//---------------------------------------------------------------------------
#include "PoittersPoint_Camera.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {
// namespace PoittersPoint

// 静的変数の初期化
float Camera::shake_timer_     = 0.0f;
float Camera::shake_intensity_ = 0.0f;

//! @brief カメラシェイクの開始
void Camera::Shake(float duration, float intensity)
{
    shake_timer_     = duration;
    shake_intensity_ = intensity;
}

//! @brief 初期化
//! @return 初期化終了
bool Camera::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetName("Camera");

    AddComponent<ComponentCamera>();    //カメラコンポーネントを付ける
    if(auto c = GetComponent<ComponentCamera>()) {
        c->SetPositionAndTarget({0, 70, 120}, {0, 0, -60});
    }

    //// プレイヤーがいるなら追従カメラにする
    //if(Scene::Object::Get<Object>("Player")) {
    //    AddComponent<ComponentSpringArm>();
    //    if(auto c = GetComponent<ComponentSpringArm>()) {
    //        c->SetSpringArmObject("Player");
    //    }
    //}

    return true;
}

//! @brief 更新
void Camera::Update()
{
    Super::Update();

    // ★ カメラシェイク処理
    if(shake_timer_ > 0.0f) {
        shake_timer_ -= GetDeltaTime();

        // -1.0 ～ 1.0 のランダム値を生成
        float offsetX = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * shake_intensity_;
        float offsetY = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * shake_intensity_;
        float offsetZ = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * shake_intensity_;

        // 基本のカメラ座標とターゲットにオフセットを加算して揺らす
        if(auto c = GetComponent<ComponentCamera>()) {
            // 基本座標（{0, 70, 120}）と基本注視点（{0, 0, -60}）にランダムな値を加算
            float3 pos    = {0.0f + offsetX, 70.0f + offsetY, 120.0f + offsetZ};
            float3 target = {0.0f + offsetX, 0.0f + offsetY, -60.0f + offsetZ};

            c->SetPositionAndTarget(pos, target);
        }

        // 時間終了時に元の位置に戻す
        if(shake_timer_ <= 0.0f) {
            shake_timer_ = 0.0f;
            if(auto c = GetComponent<ComponentCamera>()) {
                c->SetPositionAndTarget({0, 70, 120}, {0, 0, -60});
            }
        }
    }
}

}    // namespace PoittersPoint
