#include "Box.h"
#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Utils/HelperLib.h>

namespace Sample::GameSample {

BoxPtr Box::Create(const float3& pos, const float3& front)
{
    // 箱の作成
    auto box = Scene::Object::Create<Box>();

    // vecの方向に向ける
    auto mat = HelperLib::Math::CreateMatrixByFrontVector(front);
    box->SetMatrix(mat);

    // posの位置に設定
    box->SetTranslate(pos);

    return box;
}

bool Box::Init()
{
    Super::Init();

    SetName("Box");

    // モデルを使用する( 3倍の大きさとする )
    AddComponent<ComponentModel>("data/Sample/Sci-fi_Box/Sci-fi Box.mv1")->SetScaleAxisXYZ({3.0f});

    // 使用しているモデルにその形状の当たりをつける
    AddComponent<ComponentCollisionModel>()->AttachToModel(true);

    return true;
}

void Box::Update()
{
    AddTranslate({0, 0, -0.05f});
}

}    // namespace Sample::GameSample
