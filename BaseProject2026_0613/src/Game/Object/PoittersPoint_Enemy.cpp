//---------------------------------------------------------------------------
//! @file   PoittersPoint_Enemy.cpp
//! @brief  PoittersPoint_Enemy
//---------------------------------------------------------------------------
#include "PoittersPoint_Enemy.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "Game/Scene/PoittersPoint_GameOver.h";

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
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

    return true;
}

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

    if(!is_dead) {
        if(auto model = GetComponent<ComponentModel>()) {
            if(!model->IsPlaying()) {
                is_dead = true;
                SetName("deadEnemy");
            }
        }
    }
}

void Enemy::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    // 当たった相手の名前がBulletだったら消去する
    auto name = hit_info.hit_collision_->GetOwner()->GetNameDefault();

    if(name == "Bullet") {
        // 自分を削除する
        //Scene::Object::Release(SharedThis());

        // どのシーンでもエネミーは存在できるし
        // Xのシーンなら死亡数のカウントができる

        // 今のシーンを取得
        auto now_scene = Scene::GetCurrentScene();
        // sharedポインタのdynamic_cast
        // 間違っている場合nullptrが返ってくるため処理が行われない
        if(auto scene = dynamic_cast<PoittersPoint_Stage*>(now_scene)) {
            is_down = true;

            if(auto model = GetComponent<ComponentModel>()) {
                model->PlayAnimationNoSame("dead", false, 0.1f);
            }

            if(auto col = GetComponent<ComponentCollisionCapsule>()) {
                RemoveComponent<ComponentCollisionCapsule>();
            }
            /*
            // 当たり判定を無効化する
            if(auto col = GetComponent<ComponentCollisionCapsule>()) {
                col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                                          (u32)ComponentCollision::CollisionGroup::PLAYER);
            }
            */
        }

        // 弾も削除する
        Scene::Object::Release(hit_info.hit_collision_->GetOwnerPtr());
    }

    if(name == "Player") {
        //Scene::Change(Scene::GetScene<TutorialX_GameOver>());
    }

    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
