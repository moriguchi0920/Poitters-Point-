//---------------------------------------------------------------------------
//! @file   SceneAnimationOverride.cpp
//! @brief  アニメーション上書き合成サンプルシーン
//---------------------------------------------------------------------------
#include "SceneAnimationOverride.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>

#include <System/Graphics/Animation.h>
#include <System/SystemMain.h>

namespace {

std::shared_ptr<Model>             model_;                   //!< 3Dモデル
std::shared_ptr<Animation>         animation_;               //!< アニメーション
std::shared_ptr<AnimationModifier> animation_upper_body_;    //!< 上半身用アニメーション

}    // namespace

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneAnimationOverride::Init()
{
    //-------------------------------------------------------
    //! アニメーション定義
    // idle/jump/walk/walk2はアニメーションファイル内の
    // 0番目には何も入っていないため1番で登録
    //-------------------------------------------------------
    // {アニメーション名, ファイル名, ファイル内のアニメーション番号, 再生速度}
    static const Animation::Desc desc_[] = {
        {  "idle",   "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f},
        {  "jump",   "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f},
        {  "walk",   "data/Sample/Player/Anim/Walk.mv1", 1, 1.0f},
        { "walk2",  "data/Sample/Player/Anim/Walk2.mv1", 1, 1.0f},

        {"dance1", "data/Sample/Player/Anim/Dance1.mv1", 0, 1.0f},
        {"dance2", "data/Sample/Player/Anim/Dance2.mv1", 0, 1.0f},
        {"dance3", "data/Sample/Player/Anim/Dance3.mv1", 0, 1.0f},
        {"dance4", "data/Sample/Player/Anim/Dance4.mv1", 0, 1.0f},
        {"dance5", "data/Sample/Player/Anim/Dance5.mv1", 0, 1.0f},
    };

    //-------------------------------------------------------
    // モデル
    //-------------------------------------------------------
    model_ = std::make_shared<Model>("data/Sample/Player/model.mv1");

    //-------------------------------------------------------
    // アニメーション
    //-------------------------------------------------------

    // アニメーション初期化
    animation_ = std::make_shared<Animation>(desc_, std::size(desc_));

    //  モデルにアニメーションを設定
    model_->bindAnimation(animation_.get());

    //==========================================================
    // 上半身のアニメーションを上書き
    // 指定したframe_index以降の子関節が上書きされます。
    // 上半身のアニメーション制御は、ここで生成されたanimation_upper_body_で
    // 独立して再生制御できます。
    //==========================================================
    s32 frame_index = DxLib::MV1SearchFrame(*model_, "mixamorig:Spine");    // 上半身

    animation_upper_body_ = animation_->createAnimationModifier(frame_index);

    //==========================================================
    // アニメーションを再生
    //==========================================================
    bool is_loop = true;
    animation_->play("walk", is_loop, 1.0f);

    // 上書きされた上半身部分
    animation_upper_body_->play("dance4", is_loop, 1.0f);
    animation_upper_body_->isPlaying();

    //----------------------------------------------------------
    // カメラ
    //----------------------------------------------------------
    auto o = Scene::Object::Create<Object>();
    o->SetName(u8"カメラ");

    // カメラコンポーネントを追加
    auto camera = o->AddComponent<ComponentCamera>();

    constexpr f32 fov          = 60.0f;                  // 画角
    float3        eye_position = {1.5f, 1.0f, -2.0f};    // カメラの位置
    float3        look_at      = {0.0f, 0.5f, 0.0f};     // 注視点

    camera->SetPerspective(fov);
    camera->SetPositionAndTarget(eye_position, look_at);
    camera->SetCurrentCamera();

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneAnimationOverride::Update()
{
    float dt = GetDeltaTime();

    // アニメーション再生時間を進める
    animation_->update(dt);

    // モデルを更新
    model_->update(dt);
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneAnimationOverride::Draw()
{
    matrix mat_world = matrix::scale(0.01f);
    model_->setWorldMatrix(mat_world);

    model_->render();
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneAnimationOverride::Exit()
{
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneAnimationOverride::GUI()
{
}
