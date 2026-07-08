//---------------------------------------------------------------------------
//! @file   SceneVelocityBuffer.cpp
//! @brief  速度バッファサンプルシーン
//---------------------------------------------------------------------------
#include "SceneVelocityBuffer.h"
#include "System/Component/ComponentModel.h"

namespace {
// 速度バッファ生成ピクセルシェーダー
std::shared_ptr<ShaderPs> shader_ps_velocity_;
}

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool SceneVelocityBuffer::Init()
{
    //==========================================================
    // 速度バッファ出力用のシェーダーを読み込み
    //==========================================================
    shader_ps_velocity_ = std::make_shared<ShaderPs>("data/Shader/ps_model_velocity.fx");
    if(!shader_ps_velocity_) {
        return false;
    }

    //----------------------------------------------------------
    // モデル
    //----------------------------------------------------------
    {
        auto obj = Scene::Object::Create<Object>()->SetName("Model");

        //----------------
        // モデル
        auto model = obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1");

        //==========================================================
        // モデルにカスタムシェーダーを設定
        //==========================================================
        model->SetShader(nullptr, shader_ps_velocity_.get());

        //----------------
        // アニメーション
        model->SetAnimation({
            {"dance4", "data/Sample/Player/Anim/Dance4.mv1", 0, 1.0f},
        });

        // アニメーション再生開始
        bool isLoop = true;
        model->PlayAnimation("dance4", isLoop);
    }

    //----------------------------------------------------------
    // カメラ
    //----------------------------------------------------------
    {
        auto obj = Scene::Object::Create<Object>();    // カメラオブジェクト
        obj->SetName("Camera");                        // 名前設定

        auto camera = obj->AddComponent<ComponentCamera>();                         // コンポーネント
        camera->SetPositionAndTarget({0.0f, 10.0f, -20.0f}, {0.0f, 5.0f, 0.0f});    // 位置と注視点
    }

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//! @param  [in]    delta   経過時間
//---------------------------------------------------------------------------
void SceneVelocityBuffer::Update()
{
    auto obj   = Scene::Object::Get<Object>("Model");
    auto model = obj->GetComponent<ComponentModel>();

    // モデル行列を更新
    matrix mat_world = matrix::scale(0.1f);
    mat_world        = mul(mat_world, matrix::translate(float3(0.0f, 0.0f, 0.0f)));
    model->SetMatrix(mat_world);
}

//---------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------
void SceneVelocityBuffer::Draw()
{
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void SceneVelocityBuffer::Exit()
{
    //==========================================================
    // シェーダーを解放
    //==========================================================
    shader_ps_velocity_.reset();
}

//---------------------------------------------------------------------------
//! GUI表示
//---------------------------------------------------------------------------
void SceneVelocityBuffer::GUI()
{
}
