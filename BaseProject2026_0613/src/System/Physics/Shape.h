//---------------------------------------------------------------------------
//!	@file	Shape.h
//! @brief	形状
//---------------------------------------------------------------------------
#pragma once

namespace shape {
//--------------------------------------------------------------
//! デフォルト質量
//--------------------------------------------------------------
constexpr float default_density_ = 1000.0f;

//--------------------------------------------------------------
//! 形状の種類
//--------------------------------------------------------------
enum class Type : s32
{
    Unknown,       //!< 未定義
    Sphere,        //!< 球
    Box,           //!< ボックス
    Capsule,       //!< カプセル
    Cylinder,      //!< 円筒
    ConvexHull,    //!< 凸形状
    Mesh,          //!< 静的メッシュ
};

//===========================================================================
//! 形状基底
//===========================================================================
class Base
{
public:
    //! コンストラクタ
    Base(shape::Type type)
        : type_(type)
    {
    }

    //! 形状の種類を取得
    shape::Type shapeType() const { return type_; }

protected:
    shape::Type type_    = shape::Type::Unknown;
    f32         density_ = default_density_;
};

//===========================================================================
//! 球 Sphere
//===========================================================================
class Sphere : public shape::Base
{
public:
    float3 position_;    //!< 中心座標
    f32    radius_;      //!< 半径

    //! コンストラクタ
    Sphere(const float3& position, f32 radius, f32 density = default_density_)
        : shape::Base(shape::Type::Sphere)
        , position_(position)
        , radius_(radius)
    {
        density_ = density;
    }
};

//===========================================================================
//! ボックス Box
//===========================================================================
class Box : public shape::Base
{
public:
    float3 extent_;    //!< 幅

    //! コンストラクタ
    Box(const float3& extent, f32 density = default_density_)
        : shape::Base(shape::Type::Box)
        , extent_(extent)
    {
        density_ = density;
    }
};

//===========================================================================
//! カプセル Capsule
//===========================================================================
class Capsule : public shape::Base
{
public:
    f32 half_height_;    //!< 高さの半分
    f32 radius_;         //!< 半径

    //! コンストラクタ
    Capsule(f32 half_height, f32 radius, f32 density = default_density_)
        : shape::Base(shape::Type::Capsule)
        , half_height_(half_height)
        , radius_(radius)
    {
        density_ = density;
    }
};

//===========================================================================
//! 円筒 Cylinder
//===========================================================================
class Cylinder : public shape::Base
{
public:
    f32 half_height_;    //!< 高さの半分
    f32 radius_;         //!< 半径

    //! コンストラクタ
    Cylinder(f32 half_height, f32 radius, f32 density = default_density_)
        : shape::Base(shape::Type::Box)
        , half_height_(half_height)
        , radius_(radius)
    {
        density_ = density;
    }
};

//===========================================================================
//! 凸形状 ConvexHull
//===========================================================================
class ConvexHull : public shape::Base
{
public:
    std::vector<float3> vertices_;

    //! コンストラクタ
    ConvexHull(Model* model);
};

//===========================================================================
//! メッシュ Mesh
//===========================================================================
class Mesh : public shape::Base
{
public:
    std::vector<float3> vertices_;
    std::vector<u32>    indices_;

    //! コンストラクタ
    //! @param  [in]    model   モデルデーター
    //! @param  [in]    scale   スケール値
    Mesh(Model* model, f32 scale = 1.0f);
};

}    // namespace shape
