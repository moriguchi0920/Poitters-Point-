#include "Mouse.h"
#include "Enemy.h"
#include "Shot.h"
#include "../../Over/OverScene.h"

#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCamera.h>
#include <System/Component/ComponentTargetTracking.h>
#include <System/Component/ComponentSpringArm.h>

namespace Sample::GameSample {

MousePtr Mouse::Create(const float3& pos, const float3& front)
{
    // 箱の作成
    auto mouse = Scene::Object::Create<Mouse>();
    mouse->SetName("Mouse");

    // vecの方向に向ける
    auto mat = HelperLib::Math::CreateMatrixByFrontVector(front);
    mouse->SetMatrix(mat);

    // posの位置に設定
    mouse->SetTranslate(pos);

    return mouse;
}

bool Mouse::Init()    //override
{
    __super::Init();

    // モデルコンポーネント(0.08倍)
    auto model = AddComponent<ComponentModel>("data/Sample/Player/model.mv1");

    model->SetScaleAxisXYZ({0.08f});    //

    model->SetAnimation({
        {  "idle",   "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f},
        {  "jump",   "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f},
        {  "walk",   "data/Sample/Player/Anim/Walk.mv1", 1, 1.0f},
        { "walk2",  "data/Sample/Player/Anim/Walk2.mv1", 1, 1.0f},
        {"dance1", "data/Sample/Player/Anim/Dance1.mv1", 0, 1.0f},
        {"dance2", "data/Sample/Player/Anim/Dance2.mv1", 0, 1.0f},
        {"dance3", "data/Sample/Player/Anim/Dance3.mv1", 0, 1.0f},
        {"dance4", "data/Sample/Player/Anim/Dance4.mv1", 0, 1.0f},
        {"dance5", "data/Sample/Player/Anim/Dance5.mv1", 0, 1.0f}
    });

    // コリジョン(カプセル)
    auto col = AddComponent<ComponentCollisionCapsule>();    //
    col->SetTranslate({0, 0, 0});
    col->SetRadius(2.5);
    col->SetHeight(10);
    col->UseGravity();

    auto target = AddComponent<ComponentTargetTracking>();
    target->SetTrackingNode("mixamorig:Neck");
    target->SetFrontVector({0, 0, -1});

    target->SetTrackingLimitLeftRight({70, 70});

    target->SetTrackingLimitUpDown({10, 10});

    return true;
}

void Mouse::Update()    //override
{
    auto mdl        = GetComponent<ComponentModel>();
    auto near_enemy = TrackingNearEnemy(SharedThis());

    if(mdl) {
        if(auto target = GetComponent<ComponentTargetTracking>()) {
            target->SetTargetObjectPtr(near_enemy);
            //target->SetFrontVector(-mdl->GetMatrix().axisZ());
        }
    }

    float3 move = float3(0, 0, 0);

    if(IsKeyRepeat(KEY_INPUT_A)) {
        rot_y_ += -1.0f;
    }
    if(IsKeyRepeat(KEY_INPUT_D)) {
        rot_y_ += 1.0f;
    }
    SetRotationAxisXYZ({0, rot_y_, 0});

    auto mat = GetMatrix();

#ifdef USE_MOUSE_CAMERA
    // カメラ方向を取得してその方向に動かす
    auto   cam = Scene::Object::Get<Camera>("PlayerCamera");
    float3 v   = GetTranslate() - cam->GetTranslate();
    mat        = HelperLib::Math::CreateMatrixByFrontVector(-v);
#endif    //USE_MOUSE_CAMERA

    if(IsKeyRepeat(KEY_INPUT_UP)) {
        float3 vec  = mat.axisZ();
        move       += -vec;
    }
    if(IsKeyRepeat(KEY_INPUT_RIGHT)) {
        float3 vec  = mat.axisX();
        move       += -vec;
    }
    if(IsKeyRepeat(KEY_INPUT_DOWN)) {
        float3 vec  = mat.axisZ();
        move       += vec;
    }
    if(IsKeyRepeat(KEY_INPUT_LEFT)) {
        float3 vec  = mat.axisX();
        move       += vec;
    }

#if 1    // Animation
    if(mdl) {
        if(length(move).x > 0) {
            // 動いてる
            move = normalize(move);

            float x     = -move.x;
            float z     = -move.z;
            float theta = atan2(x, z) * RadToDeg - rot_y_;
            // モデルだけ回転 (ついてるカメラは回らない)
            mdl->SetRotationAxisXYZ({0, theta, 0});

            if(mdl->GetPlayAnimationName() != "walk")
                mdl->PlayAnimation("walk", true);
        }
        else {
            // 止まってる
            if(mdl->GetPlayAnimationName() != "idle")
                mdl->PlayAnimation("idle", true);
        }
    }
#endif
    move *= speed_ * GetDeltaTime60();

    // 地面移動スピードを決定する
    AddTranslate(move);

    // Shot
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        float3 vec = {0, 0, 1};

        if(mdl)
            vec = mul(-float4(mdl->GetMatrix().axisZ(), 0), GetMatrix()).xyz;

        Shot::Create(GetTranslate() + float3(0, 5, 0), vec);
    }
}

// 基本描画の後に処理します
void Mouse::LateDraw()    //override
{
    gauge_.Draw();
}

void Mouse::GUI()    //override
{
    __super::GUI();

    // GUI描画
    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();

        ImGui::DragFloat(u8"速度", &speed_, 0.01f, 0.01f, 10.0f, "%2.2f");
        ImGui::DragFloat(u8"HitPoint", &gauge_.Value(), 1, 0, 100, "%3.0f");
    }
    ImGui::End();
}

void Mouse::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)    //override
{
    // 次のownerのオブジェクトと当たった!
    auto owner = hitInfo.hit_collision_->GetOwnerPtr();
    printfDx("\nHit:%s", owner->GetName().data());

    if(owner->GetNameDefault() == "Enemy") {
        if(gauge_.Value() > 0) {
            gauge_ -= 1;
            if(gauge_ <= 0)
                Scene::Change(Scene::GetScene<OverScene>());
        }

        //Scene::ReleaseObject( owner );
    }

    // 当たりで移動させる(これが無ければめり込みます)
    __super::OnHit(hitInfo);
}

void Mouse::SetSpeed(float s)
{
    speed_ = s;
}

float Mouse::GetSpeed()
{
    return speed_;
}

float& Mouse::Speed()
{
    return speed_;
}

}    // namespace Sample::GameSample

CEREAL_REGISTER_TYPE(Sample::GameSample::Mouse)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, Sample::GameSample::Mouse)
