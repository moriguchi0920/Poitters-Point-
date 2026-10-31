#include "PoittersPoint_Player.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "Game/Component/ComponentCameraController.h"
#include "Game/Component/ComponentStateControllerWalk.h"
#include "Game/Component/ComponentStateThrow.h"
#include "Game/Component/ComponentStateMachine.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"
#include <System/Component/ComponentObjectController.h>
#include "Game/Component/ComponentHitPoints.h"

namespace PoittersPoint {

bool Player::Init()
{
    Super::Init();

    SetName("Player");

    AddComponent<ComponentModel>("data/Game/Models/Player/Player.mv1");
    if(auto model = GetComponent<ComponentModel>()) {
        model
            ->SetAnimation({
                {     "idle",        "data/Game/Models/Player/Anims/Idle.mv1", 1, 1.0f}, // Idle
                {     "walk",        "data/Game/Models/Player/Anims/Walk.mv1", 1, 1.0f}, // Walk
                {    "throw",  "data/Game/Models/Player/Anims/HandsThrow.mv1", 1, 1.0f}, // Throw
                {  "lift up", "data/Game/Models/Player/Anims/HandsLiftup.mv1", 1, 1.0f}, // liftup
                {"grab idle",    "data/Game/Models/Player/Anims/GrabIdle.mv1", 1, 1.0f}, // GrabIdle
                {"grab walk",    "data/Game/Models/Player/Anims/GrabWalk.mv1", 1, 1.0f}, // GrabWalk
        })
            ->SetScaleAxisXYZ({0.11f, 0.11f, 0.11f});
    }

    SetTranslate({0, 8, 0});

    // ステートマシン(プレイヤー入力/CPU)は生成側のStageで付与する
    // 他者に掴まれて投げられるようにする
    AddComponent<ComponentGrabbable>()->SetLiftTime(0.5f);

    // 体力を管理するコンポーネントを追加
    AddComponent<ComponentHitPoints>()->SetMaxAndCurrentHP(5.0f);

    // カプセルコリジョンのコンポーネントを追加
    AddComponent<ComponentCollisionCapsule>();
    if(auto collision = GetComponent<ComponentCollisionCapsule>()) {
        collision->SetCollisionGroup(ComponentCollision::CollisionGroup::PLAYER);
        collision->UseGravity();
        collision->SetRadius(3.8f);
        collision->SetHeight(22.0f);
    }

    return true;
}

void Player::Update()
{
    Super::Update();

    // 視線・ターゲット検出処理
    OnEyeSight();
}

void Player::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    auto hitter = hit_info.hit_collision_->GetOwner();

    // ヒットした相手が掴めるもの(=投げられて移動中)か判定
    if(auto grabbable = hitter->GetComponent<ComponentGrabbable>()) {
        // 投げた本人自身の場合はダメージなし
        if(grabbable->IsThrower(SharedThis())) {
            __super::OnHit(hit_info);
            return;
        }

        // 地面で停止中はダメージなし
        if(!grabbable->IsMoving()) {
            __super::OnHit(hit_info);
            return;
        }

        // 既にこのターゲットをHit済みならスルー(連続ヒット防止)
        if(grabbable->IsAlreadyHit(SharedThis())) {
            __super::OnHit(hit_info);
            return;
        }

        // 初Hitなら記録してダメージ処理へ
        grabbable->AddHitTarget(SharedThis());
    }
    else {
        // 弾などgrabbable以外は今まで通り
        __super::OnHit(hit_info);
        return;
    }

    // 投げ物ごとのダメージ値を適用
    if(auto hp = GetComponent<ComponentHitPoints>()) {
        hp->TakeDamage(hitter->GetComponent<ComponentGrabbable>()->GetDamage());
    }
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
                    // 視線で捕らえた対象を保持する(プレイヤーとCPU共通)
                    eyesight_target_ = target;
                    // ステートマシンがあれば掴み対象として記録する
                    // 人間はスペースキーでStartGrab、CPUはACTION_GRABの到達時にStartGrabする
                    if(auto state_machine = GetComponent<ComponentStateMachine>()) {
                        // プレイヤーよりもオブジェクトの持ち上げを優先
                        if (std::dynamic_pointer_cast<PoittersPoint::Player>(target))
                        {
                            // 判定内にオブジェクトがいない場合のみ
                            if (!state_machine->IsGrabTargetExist())
                            {
                                // 対象を持ち上げられるとして登録
                                state_machine->GrabbableHit(target);
                            }
                        }
                        // プレイヤー以外なら
                        else
                        {
                            // そのまま登録
                            state_machine->GrabbableHit(target);
                        }
                        
                    }
                }
            }
        }
    }
}

const ObjectWeakPtr& Player::GetEyeSightTarget() const
{
    return eyesight_target_;
}

void Player::SetCharacterStatus(const CharacterData& data)
{
    // 代入
    model_path_      = data.model_path_;
    anims_           = data.anims_;
    local_scale_     = data.local_scale_;
    move_speed_      = data.move_speed_;
    throw_power_     = data.throw_power_;
    lift_time_ratio_ = data.lift_time_ratio_;
    // 入力されたアニメーションとスケールをモデルに反映する
    if(auto model = GetComponent<ComponentModel>()) {
        model->SetAnimation(anims_)->SetScaleAxisXYZ(local_scale_);
    }
}

float Player::GetMoveSpeed()
{
    return move_speed_;
}

}    // namespace PoittersPoint
