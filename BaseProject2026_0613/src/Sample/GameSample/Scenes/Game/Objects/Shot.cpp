#include "Shot.h"
#include <System/Scene.h>
#include <System/Component/ComponentCollisionSphere.h>

namespace Sample::GameSample {

ShotPtr Shot::Create(const float3& pos, const float3& vec)
{
    // 弾の作成
    auto shot = Scene::Object::Create<Shot>();

    shot->SetPositionAndDirection(pos, vec);

    return shot;
}

bool Shot::Init()
{
    Super::Init();

    SetName("Shot");

    //
    AddComponent<ComponentCollisionSphere>()                    //< 球コリジョン
        ->SetRadius(radius_)                                    //< 半径設定
        ->SetHitCollisionGroup(                                 //< 当たりるグループ
            (u32)ComponentCollision::CollisionGroup::ENEMY |    //< 敵
            (u32)ComponentCollision::CollisionGroup::GROUND     //< グラウンド
        );
    return true;
}

void Shot::Update()
{
    // スピードに合わせて移動 AddTranslateには DeltaSecondの掛け合わせは不要
    AddTranslate(vec_ * speed_);
}

void Shot::Draw()
{
    DrawSphere3D(cast(GetTranslate()), radius_, 10, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);
}

void Shot::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
{
    // 敵に当たったら敵を消す
    auto owner = hitInfo.hit_collision_->GetOwnerPtr();
    if(owner->GetNameDefault() == "Enemy") {
        Scene::Object::Release(owner);
        Scene::Object::Release(SharedThis());
    }

    // 地面に当たったら自分だけ消す
    if(owner->GetNameDefault() == "Ground") {
        Scene::Object::Release(SharedThis());
    }
}

void Shot::SetPositionAndDirection(const float3& pos, const float3& vec)
{
    // vecの方向に向ける
    auto mat = HelperLib::Math::CreateMatrixByFrontVector(vec);
    SetMatrix(mat);

    // 進む方向を向きの方向にする
    vec_ = normalize(vec);

    // posの位置に設定
    SetTranslate(pos);
}

}    // namespace Sample::GameSample
