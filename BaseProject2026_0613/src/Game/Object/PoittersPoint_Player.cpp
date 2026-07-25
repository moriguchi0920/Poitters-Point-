//---------------------------------------------------------------------------
//! @file   PoittersPoint_Player.cpp
//! @brief  PoittersPoint_Player
//---------------------------------------------------------------------------
#include "PoittersPoint_Player.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "Game/Component/ComponentCameraController.h"
#include "Game/Component/State/ComponentStateControllerWalk.h"
#include "Game/Component/State/ComponentStateThrow.h"
#include "Game/Component/StateMachine/ComponentPlayerState.h"
#include "Game/Component/StateMachine/ComponentStateMachine.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
bool Player::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetName("Player");
    //auto player = Scene::Object::Create<Object>("Player");

    AddComponent<ComponentModel>("data/Game/Models/Player/Player.mv1");
    if(auto model = GetComponent<ComponentModel>()) {
        model
            ->SetAnimation({
                //                                  モデルビユーで何番目にいたか
                //                                                 ↓
                {     "idle",        "data/Game/Models/Player/Anims/Idle.mv1", 1, 1.0f}, // Idle
                {     "walk",        "data/Game/Models/Player/Anims/Walk.mv1", 1, 1.0f}, // Walk
                {    "throw",  "data/Game/Models/Player/Anims/HandsThrow.mv1", 1, 1.0f}, // Throw
                {  "lift up", "data/Game/Models/Player/Anims/HandsLiftup.mv1", 1, 1.0f}, // liftup
                {"grab idle",    "data/Game/Models/Player/Anims/GrabIdle.mv1", 1, 1.0f}, // Throw
                {"grab walk",    "data/Game/Models/Player/Anims/GrabWalk.mv1", 1, 1.0f}, // Throw
        })
            //->SetScaleAxisXYZ({0.12f, 0.12f, 0.12f});
            ->SetScaleAxisXYZ({0.11f, 0.11f, 0.11f});
    }

    SetTranslate({0, 8, 0});

    AddComponent<ComponentPlayerState>();

    // カメラの制御を行うコンポーネントを追加
    //AddComponent<ComponentCameraController>();

    // カプセルコリジョンのコンポーネントを追加
    AddComponent<ComponentCollisionCapsule>();
    if(auto collision = GetComponent<ComponentCollisionCapsule>()) {
        // 所属するグループを「PLAYER」とします
        collision->SetCollisionGroup(ComponentCollision::CollisionGroup::PLAYER);

        // Collisionの重力を有効にします
        collision->UseGravity();
        collision->SetRadius(3.8f);     // コリジョンの半径を3.0 にする
        collision->SetHeight(22.0f);    // コリジョンの高さを13.0 にする
    }

    return true;
}

void Player::Update()
{
    OnEyeSight();
}

void Player::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    //auto target = hit_info.hit_collision_->GetOwnerPtr();
    //if(auto grabbable = target->GetComponent<ComponentGrabbable>()) {
    //    auto player_state = GetComponent<ComponentPlayerState>();
    //    if(player_state->can_grab_ && grabbable->GetCanGrab()) {
    //        player_state->GrabbableHit(target);
    //    }
    //}

    __super::OnHit(hit_info);
}

void Player::OnEyeSight()
{
    auto   ObjArray = Scene::Object::GetArray<Object>();
    float3 vec;
    if(auto model = GetComponent<ComponentModel>()) {
        vec = -model->GetWorldVectorAxisZ();
        normalize(vec);
    }

    // 親から敵へのベクトル
    float3 targetVec = {0, 0, 0};
    // 一番近い敵への距離を保存する変数
    float shortest = 1000.0f;
    // 範囲for
    for(auto& obj : ObjArray) {
        if(obj == static_cast<ObjectPtr>(shared_from_this())) {
            continue;
        }

        // 親の座標(斜めに当てたいので上に上げて判定)
        float3 pos = GetTranslate() - vec * 0.5f + float3(0.0f, 1.0f, 0.0f);
        // 敵の座標
        float3 targetPos = obj->GetTranslate();

        // ベクトルを取得
        targetVec = targetPos - pos;
        // 親の正面ベクトルと敵へのベクトルの内積をとる
        float targetDot = dot(normalize(vec), normalize(targetVec));
        // 正規化ベクトル同士の内積なのでarccosで角度がとれる
        float angle = acosf(targetDot);
        // スカラー付きベクトルの長さは各要素の二乗を足したものの平方根でとれる
        float distance = sqrtf(targetVec.x * targetVec.x + targetVec.y * targetVec.y + targetVec.z * targetVec.z);
        // 内積と距離をかけて円錐の範囲にいるかをとる
        float dotDis = distance * targetDot;

        // 最短距離なら
        if(distance <= shortest) {
            // 最短距離を更新
            shortest = distance;
            // 円錐の角度内か
            bool isInAngle = angle <= 3.14f * 0.45 && -3.14f * 0.45 <= angle;
            // 距離は円錐の範囲内か
            bool isInDistance = dotDis <= 50.0f;

            // 両方満たしていたら
            if(isInAngle && isInDistance) {
                auto target = obj;
                if(auto grabbable = target->GetComponent<ComponentGrabbable>()) {
                    auto player_state = GetComponent<ComponentPlayerState>();
                    if(player_state->can_grab_ && grabbable->GetCanGrab()) {
                        player_state->GrabbableHit(target);
                    }
                }
            }
        }
    }
}

}    // namespace PoittersPoint
