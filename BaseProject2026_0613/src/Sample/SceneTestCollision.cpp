#include "SceneTestCollision.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCamera.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionBox.h>
#include <System/Component/ComponentCollisionLine.h>

// Objectを継承してOnHitをオーバーライドすることで
// 当たった時に爆発させたり、ダメージを与えたりできる
class ObjectMouse : public Object
{
public:
    bool Init()
    {
        __super::Init();

        SetName("Mouse1");
        SetTranslate({-1, 0, 0});

        // モデルコンポーネント(0.05倍)
        AddComponent<ComponentModel>("data/Sample/Player/model.mv1")->SetScaleAxisXYZ({0.05f});

        // コリジョン(カプセル)
        auto col1 = AddComponent<ComponentCollisionCapsule>()    //< コリジョン
                        ->SetTranslate({0, 0, 0})
                        ->SetHeight(6.0f);

        col1->SetName("Body");
        col1->UseGravity();

        // コリジョン(Sphere) 2つめの当たり(当たりのコンポーネントは複数持てます)
        auto col2 = AddComponent<ComponentCollisionSphere>()    //< コリジョン
                        ->SetTranslate({0, 0, 0})
                        ->SetRadius(1);

        // モデルに12番のノードにアタッチします
        col2->AttachToModel(12);
        col2->SetName("Hand");

        // コリジョン(Sphere) 3つめの当たり(当たりのコンポーネントは複数持てます)
        auto col3 = AddComponent<ComponentCollisionLine>()    //< コリジョン
                        ->SetTranslate({0, 0, 0})
                        ->SetLine({0, 1, 0}, {0, 1, -8})
                        ->SetName("Line");

        SetStatus(StatusBit::Located, false);

        return true;
    }

    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override
    {
        // 次のownerのオブジェクトと当たった!
        auto owner1   = hitInfo.collision_->GetOwnerPtr();
        auto colname1 = hitInfo.collision_->GetName();

        auto owner2   = hitInfo.hit_collision_->GetOwnerPtr();
        auto colname2 = hitInfo.hit_collision_->GetName();
        printfDx("\nHit %s(%s) <=> %s(%s)", owner1->GetName().data(), colname1.data(), owner2->GetName().data(), colname2.data());

        // 当たりで移動させる(これが無ければめり込みます)
        __super::OnHit(hitInfo);
    }
};

bool SceneTestCollision::Init()
{
    //----------------------------------------------------------------------------------
    // CAMERA
    //----------------------------------------------------------------------------------
#if 1
    auto cam = Scene::Object::Create<Object>()
                   ->SetName("Camera")                  //< Name
                   ->AddComponent<ComponentCamera>()    //< Camera
                   ->SetPositionAndTarget({0, 20, -30}, {0, 0, 0});
#endif

    //----------------------------------------------------------------------------------
    // OBJECT1
    //----------------------------------------------------------------------------------
    // オブジェクト作成
    Scene::Object::Create<ObjectMouse>()->SetTranslate({-3, 10, 0})
        //		->AddComponent<ComponentCamera>()	//< Camera
        //		->SetPositionAndTarget( { 0, 20, -30 }, { 0, 0, 0 } )
        ;

    //----------------------------------------------------------------------------------
    // OBJECT2
    //----------------------------------------------------------------------------------
    {
        // TestObj / 位置(3,0,0)
        auto obj = Scene::Object::Create<Object>()->SetName("TestObj")->SetTranslate({3, 0, 0});

        // TestObj、モデルコンポーネント(0.08倍)
        auto comp = obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1")->SetScaleAxisXYZ({0.08f});

        // コリジョン(カプセル)
        obj->AddComponent<ComponentCollisionCapsule>()->SetTranslate({0, 0, 0})->SetHeight(6.0)->SetRadius(1.5);
    }

    //----------------------------------------------------------------------------------
    // OBJECT3
    //----------------------------------------------------------------------------------
    {
        // TestObj / 位置(3,0,0)
        auto obj = Scene::Object::Create<Object>()->SetName("Box")->SetTranslate({-12, 5, -5});

        // TestObj、コリジョンBOX
        obj->AddComponent<ComponentCollisionBox>()->SetSize({2, 2, 2})->UseGravity();
    }

    //----------------------------------------------------------------------------------
    // OBJECT Map
    //----------------------------------------------------------------------------------
    {
        auto obj = Scene::Object::Create<Object>()->SetName("Ground");
        //obj->AddComponent<ComponentModel>( "data/Sample/SwordBout/Stage/Stage00.mv1" );
        obj->AddComponent<ComponentModel>("data/Sample/FPS_Industrial/Map.mv1")->SetScaleAxisXYZ({2, 2, 2});
        //obj->AddComponent<ComponentModel>( "data/Sample/空色町1.52/sorairo1.52.mv1" )->SetScaleAxisXYZ( { 2, 2, 2 } );

        // モデルのコリジョン設定
        obj->AddComponent<ComponentCollisionModel>()->AttachToModel();
    }
    return true;
}

void SceneTestCollision::Update()
{
    auto obj = Scene::Object::Get<ObjectMouse>();
    if(IsKeyRepeat(KEY_INPUT_UP)) {
        obj->AddTranslate({0, 0, 0.3});
    }
    if(IsKeyRepeat(KEY_INPUT_DOWN)) {
        obj->AddTranslate({0, 0, -0.3});
    }
    if(IsKeyRepeat(KEY_INPUT_RIGHT)) {
        obj->AddTranslate({0.3, 0, 0});
    }
    if(IsKeyRepeat(KEY_INPUT_LEFT)) {
        obj->AddTranslate({-0.3, 0, 0});
    }
}

void SceneTestCollision::Draw()
{
    // とりあえずTitleという文字を表示しておく
    DrawFormatString(100, 50, GetColor(255, 255, 255), "Collision");
}

void SceneTestCollision::Exit()
{
    // タイトル終了時に行いたいことは今はない
}

void SceneTestCollision::GUI()
{
}
