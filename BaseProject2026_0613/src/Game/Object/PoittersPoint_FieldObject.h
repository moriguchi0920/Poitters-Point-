#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Ground.h
//! @brief  PoittersPoint_Ground
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

struct FieldObjectData
{
    std::string model_path_;
    float3      local_scale_;
};

USING_PTR(FieldObject);
class FieldObject : public Object
{
public:
    BP_OBJECT_DECL(FieldObject, "PoittersPoint::Ground");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    void SetFieldObjectStatus();

private:
    // モデルのファイルパス
    std::string model_path_;
    // モデルのサイズが合わないときのサイズ調整用
    float3 local_scale_;
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::FieldObject);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::FieldObject);
