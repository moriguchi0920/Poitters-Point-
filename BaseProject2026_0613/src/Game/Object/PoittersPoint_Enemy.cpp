//---------------------------------------------------------------------------
//! @file   PoittersPoint_Enemy.cpp
//! @brief  PoittersPoint_Enemy
//---------------------------------------------------------------------------
#include "PoittersPoint_Enemy.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "Game/Scene/PoittersPoint_GameOver.h";

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentHitPoints.h>
#include <Game/Component/ComponentStateDead.h>

namespace PoittersPoint {
// namespace PoittersPoint

//============================================================================
// 初期化処理
//============================================================================
bool Enemy::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetName("Enemy");
    //auto player = Scene::Object::Create<Object>("Player");

    AddComponent<ComponentModel>("data/Game/Models/Enemy/Enemy.mv1");
    if(auto model = GetComponent<ComponentModel>()) {
        model
            ->SetAnimation({
                {"walk",  "data/Game/Models/Enemy/Anims/Walk.mv1", 1, 1.0f}, // Walk
                { "run",   "data/Game/Models/Enemy/Anims/Run.mv1", 1, 1.0f}, // Run
                {"dead", "data/Game/Models/Enemy/Anims/Death.mv1", 1, 1.0f}, // dead
        })
            ->PlayAnimation("walk", true);

        model->SetScaleAxisXYZ({0.12f, 0.12f, 0.12f});
    }

    // ラムダ式
    auto randomRange = [](int start, int end) { return start + rand() % ((end - start) + 1); };

    //SetTranslate({30, 5, -30});
    SetTranslate({randomRange(-150, 150), 50, randomRange(-150, 150)});

    AddComponent<ComponentCollisionCapsule>()
        // 所属するグループを「ENEMY」とします
        ->SetCollisionGroup(ComponentCollision::CollisionGroup::ENEMY)
        ->SetRadius(4.0f)     // コリジョンの半径を4.0 にする
        ->SetHeight(23.0f)    // コリジョンの高さを23.0 にする
        ->UseGravity();       // Collisionの重力を有効にします

    //↑ if(auto collision = GetComponent<ComponentCollisionCapsule>()) {
    //   は処理が増えるので削除

    //AddComponent<ComponentObjectController>();
    //if(auto ctl = GetComponent<ComponentObjectController>()) {
    //    ctl->SetMoveSpeed(0.3f);
    //    ctl->SetRotateSpeed(20.0f);
    //}

    // 掴まれて投げられるようにする
    auto grabbable = AddComponent<ComponentGrabbable>();
    grabbable->SetLiftTime(0.5f);
    grabbable->SetDamage(1.0f);

    AddComponent<ComponentHitPoints>()->SetMaxAndCurrentHP(2.0f);

    return true;
}

//============================================================================
// 更新処理
//============================================================================
void Enemy::Update()
{
    __super::Update();

    //-------------------------------------------------------------------------------
    // 敵の方向をプレイヤーのほうに向ける　
    //-------------------------------------------------------------------------------
    auto player = Scene::Object::Get<Object>("Player");

    // プレイヤーとエネミーが存在している時のみ
    if(player && is_down == false && is_dead == false) {
        float3 vec    = player->GetTranslate() - GetTranslate();
        auto   length = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        auto   model  = GetComponent<ComponentModel>();
        // プレイヤーの方向に、敵の向きを最大3度回転させます
        SetRotationToPositionWithLimit(player->GetTranslate(), 3.0f);

        //==========================================
        // 状態の更新
        //==========================================
        if(is_running) {
            // 現在走っているなら、十分に近づく(35.0未満)までは「走り」を維持する
            if(length < 35.0f) {
                is_running = false;    // 歩き状態へ移行
            }
        }
        else {
            // 現在歩いているなら、十分に離れる(45.0以上)までは「歩き」を維持する
            if(45.0f <= length) {
                is_running = true;    // 走り状態へ移行
            }
        }

        //==========================================
        // 状態に応じたアニメーションと移動の実行
        //==========================================
        if(is_running) {
            model->PlayAnimationNoSame("run", true, 0.7f);
            // 向きに移動させる
            AddTranslate({0, 0, -0.3f}, true);
        }
        else {
            model->PlayAnimationNoSame("walk", true, 0.5f);
            // 向きに移動させる
            AddTranslate({0, 0, -0.1f}, true);
        }

        /*
        // これだと距離の中間点でアニメーションの連続遷移でガタつく
        if(40.0f <= length) {
            model->PlayAnimationNoSame("run", true, 1.5f);
            // 向きに移動させる
            AddTranslate({0, 0, -0.3}, true);
        }
        else if(length < 40.0f) {
            model->PlayAnimationNoSame("walk", true, 1.5f);
            // 向きに移動させる
            AddTranslate({0, 0, -0.1}, true);
        }
        */
    }

    // 死亡アニメーション終了時にis_deadをtrueにする
    // 死亡処理自体はComponentStateDeadで行う
    if(!is_dead) {
        if(auto model = GetComponent<ComponentModel>()) {
            if(!model->IsPlaying()) {
                is_dead = true;
            }
        }
    }
}

//============================================================================
// 当たり判定のコールバック
//============================================================================
void Enemy::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    auto hitter = hit_info.hit_collision_->GetOwner();

    // ヒットした相手が掴めるものなら
    if(auto grabbable = hitter->GetComponent<ComponentGrabbable>()) {
        // 投げた本人自身の場合はダメージなし
        if(grabbable->IsThrower(SharedThis())) {
            Super::OnHit(hit_info);
            return;
        }

        // 地面で停止中はダメージなし
        if(!grabbable->IsMoving()) {
            Super::OnHit(hit_info);
            return;
        }

        // 連続ヒット防止のために既にこの敵をHit済みならスルー
        if(grabbable->IsAlreadyHit(SharedThis())) {
            Super::OnHit(hit_info);
            return;
        }

        // 初Hitなら記録してダメージ処理へ
        grabbable->AddHitTarget(SharedThis());
    }
    else {
        // 弾などgrabbable以外は今まで通りのHit処理を行う
        Super::OnHit(hit_info);
        return;
    }

    // 今のシーンを取得
    auto now_scene = Scene::GetCurrentScene();
    // sharedポインタのdynamic_cast
    // 間違っている場合nullptrが返ってくるため処理が行われない
    if(auto scene = dynamic_cast<PoittersPoint_Stage*>(now_scene)) {
        if(auto hp = GetComponent<ComponentHitPoints>()) {
            // 投げれる物ごとダメージを与える
            hp->TakeDamage(hitter->GetComponent<ComponentGrabbable>()->GetDamage());

            if(hp->GetHitPoints() <= 0.0f) {
                is_down = true;

                // 死亡状態へ
                if(auto model = GetComponent<ComponentModel>()) {
                    AddComponent<ComponentStateDead>();
                }

                if(auto col = GetComponent<ComponentCollisionCapsule>()) {
                    RemoveComponent<ComponentCollisionCapsule>();
                }
            }
        }
    }

    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
