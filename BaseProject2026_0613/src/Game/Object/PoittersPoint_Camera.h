#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Camera.h
//! @brief  PoittersPoint_Camera
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Camera);
class Camera : public Object
{
public:
    BP_OBJECT_DECL(Camera, "PoittersPoint::Camera");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief 更新処理
    void Update() override;

    //! @brief カメラシェイクを開始する
    //! @param duration 揺れる時間（秒）
    //! @param intensity 揺れの強さ（振幅）
    static void Shake(float duration = 0.2f, float intensity = 1.0f);

private:
    // カメラシェイク用
    static float shake_timer_;        //!< シェイク残り時間
    static float shake_intensity_;    //!< 揺れの強さ
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Camera);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Camera);
