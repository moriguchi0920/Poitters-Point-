//---------------------------------------------------------------------------
//! @file   PoittersPoint_Enemy.cpp
//! @brief  PoittersPoint_Enemy
//---------------------------------------------------------------------------
#include "PoittersPoint_Enemy.h"
#include "PoittersPoint_Player.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentHitPoints.h>
#include <Game/Component/ComponentStateDead.h>

#include <Game/Component/ComponentCharacterPhysics.h>

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

    // 原点 (0, 0, 0) を中心とした半径 50 以内の円形範囲にランダム配置
    float theta = ((float)rand() / RAND_MAX) * DX_TWO_PI_F;    // 0 ～ 2π のランダムな角度
    float r     = sqrtf((float)rand() / RAND_MAX) * 50.0f;     // 0 ～ 50 のランダムな半径（面積が均等になるように調整）

    // 物理ボディは作成時の位置に置かれるため、先に位置を決めておく(Y軸の高さは50から落下させる想定)
    SetTranslate({cosf(theta) * r, 50.0f, sinf(theta) * r});

    // 当たり判定・重力・移動・吹き飛びはキャラクター共通の物理移動に任せる
    AddComponent<ComponentCharacterPhysics>()->Setup(4.0f, 23.0f, ComponentPhysics::CollisionType::ENEMY);

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
    // 一番近いキャラクターを狙う(名前 "Player" で探すと、複数いる時にどれが返るか分からない)
    ObjectPtr player  = nullptr;
    float     nearest = FLT_MAX;
    for(auto& character : Scene::Object::GetArray<Player>()) {
        float d = (float)length(character->GetTranslate() - GetTranslate());
        if(d < nearest) {
            nearest = d;
            player  = character;
        }
    }
    auto mover  = GetComponent<ComponentCharacterPhysics>();

    // HPが無くなったら死亡状態へ(ダメージ自体は投げ物側の ComponentGrabbable が与える)
    if(!is_down) {
        if(auto hp = GetComponent<ComponentHitPoints>(); hp && hp->IsDead()) {
            is_down = true;
            AddComponent<ComponentStateDead>();
        }
    }

    // 吹き飛ばされた後、着地して止まったら操作を戻す
    if(mover && !mover->IsControlEnabled() && mover->IsGrounded() && (float)length(mover->GetVelocity()) < 5.0f)
        mover->SetControlEnabled(true);

    // プレイヤーとエネミーが存在している時のみ
    if(player && mover && is_down == false && is_dead == false) {
        float3 vec    = player->GetTranslate() - GetTranslate();
        auto   length = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        auto   model  = GetComponent<ComponentModel>();
        // プレイヤーの方向に、敵(モデル)の向きを最大3度回転させます
        // (オブジェクト自体の回転は物理ボディで固定されているため、向きはモデルで表現する)
        vec.y = 0.0f;
        if((float)sqrtf(vec.x * vec.x + vec.z * vec.z) > 0.001f)
            model->SetRotationToVectorWithLimit(normalize(vec), 3.0f);

        // モデルの正面(-Z)方向
        float3 forward     = -model->GetWorldVectorAxisZ();
        forward.y          = 0.0f;
        float forward_len  = sqrtf(forward.x * forward.x + forward.z * forward.z);
        forward            = forward_len > 0.0001f ? forward / forward_len : float3(0.0f, 0.0f, 0.0f);

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
            mover->Move(forward * 0.3f);
        }
        else {
            model->PlayAnimationNoSame("walk", true, 0.5f);
            // 向きに移動させる
            mover->Move(forward * 0.1f);
        }

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

}    // namespace PoittersPoint
