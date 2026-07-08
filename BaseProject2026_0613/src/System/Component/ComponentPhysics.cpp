//---------------------------------------------------------------------------
//! @file   ComponentPhysics.cpp
//! @brief  Physicsコンポーネント
//---------------------------------------------------------------------------

#include <System/Scene.h>
#include <System/Component/ComponentPhysics.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentHitInfo.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/Body.h>

#include <System/Physics/PhysicsEngine.h>
#include <System/Physics/PhysicsLayer.h>
#include <System/Physics/RigidBody.h>
#include <System/Physics/Shape.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Renderer/DebugRendererRecorder.h>
#include <Jolt/Renderer/DebugRendererPlayback.h>
#include <Jolt/Physics/Body/MotionType.h>

#include <Jolt/Physics/Collision/CollideShape.h>

#include <algorithm>
#include <unordered_map>

namespace {
std::unordered_map<JPH::BodyID, ComponentPhysicsWeakPtr> collisions_;

constexpr const char* items[] = {
    "NONE",
    "BOX",
    "SPHERE",
    "CAPSULE",
    "CYLINDER",
    "MESH",
};
constexpr const char* col_types[] = {
    "WALL",
    "GROUND",
    "PLAYER",
    "ENEMY",
    "WEAPON",
    "ITEM",
    "CAMERA",

    "ETC(_0)",
    "ETC_1",
    "ETC_2",
    "ETC_3",
    "ETC_4",
    "ETC_5",
    "ETC_6",
    "ETC_7",
    "NOHIT",    //!< これがあると強制的に当たらなくなる
};

const unsigned int physics_color = GetColor(64, 64, 192);

bool initialized = false;

std::mutex mutex_infos_;

// 共通の設定
#if 0    // 内部のbroad_phase_layer_interface_設定が変更できないため、これが使えない…。
	void _layerSettings()
	{
		//physics::Engine::instance()->setLayerAlias( myLayerTable_, std::size( myLayerTable_ ) );
		physics::Engine::instance()->overrideLayerCollide( []( u16 o1, u16 o2 ) -> bool {
			// 当たる場合trueで返す
			if( ComponentPhysics::GetHitCollision( (ComponentPhysics::CollisionType)o1 ) & ( 1 << o2 ) )
				return true;

			return false;
		} );
	}
#endif

class CPContactListener : public JPH::ContactListener
{
public:
    //! コンタクトを有効にするかどうかの判定
    virtual JPH::ValidateResult OnContactValidate([[maybe_unused]] const JPH::Body&               body1,
                                                  [[maybe_unused]] const JPH::Body&               body2,
                                                  [[maybe_unused]] JPH::RVec3Arg                  baseOffset,
                                                  [[maybe_unused]] const JPH::CollideShapeResult& collision_result) override
    {
        // std::cout << "コールバックが有効か確認." << std::endl;
        ComponentPhysicsWeakPtr pd1w = collisions_.at(body1.GetID());
        ComponentPhysicsWeakPtr pd2w = collisions_.at(body2.GetID());
        ComponentPhysicsPtr     pd1  = pd1w.lock();
        ComponentPhysicsPtr     pd2  = pd2w.lock();
        if(pd1 == nullptr)
            return JPH::ValidateResult::RejectContact;
        if(pd2 == nullptr)
            return JPH::ValidateResult::RejectContact;

        ComponentPhysics::CollisionType ct1 = pd1->GetCollisionType();
        ComponentPhysics::CollisionType ct2 = pd2->GetCollisionType();

        //broad_phase_layer_interfaceでやるべきことをここで行う todo broad_phase_layer_interfaceの変更
        if(!(ComponentPhysics::GetHitCollision(ct1) & (u16)ct2) || !(ComponentPhysics::GetHitCollision(ct2) & (u16)ct1)) {
            // ヒットしない
            return JPH::ValidateResult::RejectAllContactsForThisBodyPair;
        }

        // どちらかの指定でオーバーラップするか?
        if(pd1->GetOverlapCollision() & (u16)ct2 || pd2->GetOverlapCollision() & (u16)ct1) {
            return JPH::ValidateResult::RejectAllContactsForThisBodyPair;
        }
        pd1->AddHitInfosCount();
        pd2->AddHitInfosCount();
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const JPH::Body&            inBody1,
                                const JPH::Body&            inBody2,
                                const JPH::ContactManifold& inManifold,
                                JPH::ContactSettings& /*ioSettings*/)
    {
        ComponentPhysicsWeakPtr pd1w = collisions_.at(inBody1.GetID());
        ComponentPhysicsWeakPtr pd2w = collisions_.at(inBody2.GetID());
        ComponentPhysicsPtr     pd1  = pd1w.lock();
        ComponentPhysicsPtr     pd2  = pd2w.lock();
        if(pd1 == nullptr)
            return;
        if(pd2 == nullptr)
            return;

        {
            ComponentPhysics::HitInfo info;
            info.hit_           = true;
            info.collision_     = pd1.get();
            info.hit_collision_ = pd2.get();

            JPH::Vec3 pos = {0, 0, 0};
            for(auto& px : inManifold.mRelativeContactPointsOn1) pos += inBody1.GetCenterOfMassPosition() + px;
            if(inManifold.mRelativeContactPointsOn1.size() > 0)
                pos /= (float)inManifold.mRelativeContactPointsOn1.size();

            info.hit_position_.x = pos.GetX();
            info.hit_position_.y = pos.GetY();
            info.hit_position_.z = pos.GetZ();
            //pd1->OnHit( info );		// ここで呼ぶことができません
            std::scoped_lock lock(mutex_infos_);
            pd1->SetHitInfo(info);
        }
        {
            ComponentPhysics::HitInfo info;
            info.hit_           = true;
            info.collision_     = pd2.get();
            info.hit_collision_ = pd1.get();

            JPH::Vec3 pos = {0, 0, 0};
            for(auto& px : inManifold.mRelativeContactPointsOn2) pos += inBody2.GetCenterOfMassPosition() + px;
            if(inManifold.mRelativeContactPointsOn2.size() > 0)
                pos /= (float)inManifold.mRelativeContactPointsOn2.size();

            info.hit_position_.x = pos.GetX();
            info.hit_position_.y = pos.GetY();
            info.hit_position_.z = pos.GetZ();
            //pd2->OnHit( info );		// ここで呼ぶことができません
            std::scoped_lock lock(mutex_infos_);
            pd2->SetHitInfo(info);
        }
    }

    virtual void OnContactPersisted(const JPH::Body&            inBody1,
                                    const JPH::Body&            inBody2,
                                    const JPH::ContactManifold& inManifold,
                                    JPH::ContactSettings& /*ioSettings*/)
    {
        ComponentPhysicsWeakPtr pd1w = collisions_.at(inBody1.GetID());
        ComponentPhysicsWeakPtr pd2w = collisions_.at(inBody2.GetID());
        ComponentPhysicsPtr     pd1  = pd1w.lock();
        ComponentPhysicsPtr     pd2  = pd2w.lock();
        if(pd1 == nullptr)
            return;
        if(pd2 == nullptr)
            return;

        {
            ComponentPhysics::HitInfo info;
            info.hit_           = true;
            info.collision_     = pd1.get();
            info.hit_collision_ = pd2.get();

            JPH::Vec3 pos = {0, 0, 0};
            for(auto& px : inManifold.mRelativeContactPointsOn1) pos += inBody1.GetCenterOfMassPosition() + px;
            if(inManifold.mRelativeContactPointsOn1.size() > 0)
                pos /= (float)inManifold.mRelativeContactPointsOn1.size();

            info.hit_position_.x = pos.GetX();
            info.hit_position_.y = pos.GetY();
            info.hit_position_.z = pos.GetZ();
            //pd1->OnHit( info );		// ここで呼ぶことができません
            std::scoped_lock lock(mutex_infos_);
            pd1->SetHitInfo(info);
        }
        {
            ComponentPhysics::HitInfo info;
            info.hit_           = true;
            info.collision_     = pd2.get();
            info.hit_collision_ = pd1.get();

            JPH::Vec3 pos = {0, 0, 0};
            for(auto& px : inManifold.mRelativeContactPointsOn2) pos += inBody2.GetCenterOfMassPosition() + px;
            if(inManifold.mRelativeContactPointsOn2.size() > 0)
                pos /= (float)inManifold.mRelativeContactPointsOn2.size();

            info.hit_position_.x = pos.GetX();
            info.hit_position_.y = pos.GetY();
            info.hit_position_.z = pos.GetZ();
            //pd2->OnHit( info );		// ここで呼ぶことができません
            std::scoped_lock lock(mutex_infos_);
            pd2->SetHitInfo(info);
        }
    }
};

CPContactListener _contactlistener;

void DrawBox3D(const matrix& world, float3 size)
{
    std::vector<float4> vertex = {
        { size.x,  size.y, -size.z, 1},
        { size.x, -size.y, -size.z, 1},
        {-size.x, -size.y, -size.z, 1},
        {-size.x,  size.y, -size.z, 1},
        { size.x,  size.y,  size.z, 1},
        { size.x, -size.y,  size.z, 1},
        {-size.x, -size.y,  size.z, 1},
        {-size.x,  size.y,  size.z, 1},
    };
    static std::vector<int4> index = {
        {3, 2, 1, 0},
        {5, 6, 7, 4},
        {1, 5, 4, 0},
        {2, 6, 5, 1},
        {3, 7, 6, 2},
        {7, 3, 0, 4},
    };

    for(int i = 0; i < index.size(); i++) {
        auto ix  = index[i];
        auto pt1 = mul(vertex[ix.x], world);
        auto pt2 = mul(vertex[ix.y], world);
        auto pt3 = mul(vertex[ix.z], world);
        auto pt4 = mul(vertex[ix.w], world);

        DrawLine3D(cast(pt1.xyz), cast(pt2.xyz), physics_color);
        DrawLine3D(cast(pt2.xyz), cast(pt3.xyz), physics_color);
        DrawLine3D(cast(pt3.xyz), cast(pt4.xyz), physics_color);
        DrawLine3D(cast(pt4.xyz), cast(pt1.xyz), physics_color);
    }
}

JPH::BodyID GetBodyID(const std::shared_ptr<physics::RigidBody>& body)
{
    return JPH::BodyID((u32)body->bodyID());
}
#if 1

class JPH_DEBUG_RENDERER_EXPORT BPDebugRenderer final : public JPH::DebugRenderer
{
public:
    /* JPH_OVERRIDE_NEW_DELETE */    // ワーニングが出るため展開
    JPH_INLINE void* operator new(size_t inCount) { return JPH::Allocate(inCount); }
    JPH_INLINE void  operator delete(void* inPointer) noexcept { JPH::Free(inPointer); }
    JPH_INLINE void* operator new[](size_t inCount) { return JPH::Allocate(inCount); }
    JPH_INLINE void  operator delete[](void* inPointer) noexcept { JPH::Free(inPointer); }
    JPH_INLINE void* operator new(size_t inCount, std::align_val_t inAlignment) { return JPH::AlignedAllocate(inCount, static_cast<size_t>(inAlignment)); }
    JPH_INLINE void  operator delete(void* inPointer, std::align_val_t inAlignment) noexcept
    {
        inAlignment;    // no use...
        JPH::AlignedFree(inPointer);
    }
    JPH_INLINE void* operator new[](size_t inCount, std::align_val_t inAlignment) { return JPH::AlignedAllocate(inCount, static_cast<size_t>(inAlignment)); }
    JPH_INLINE void  operator delete[](void* inPointer, std::align_val_t inAlignment) noexcept
    {
        inAlignment;    // no use...
        JPH::AlignedFree(inPointer);
    }

    struct PolygonData
    {
        VERTEX3D*   vertex       = nullptr;
        JPH::uint32 vertex_count = 0;

        u16*        index       = nullptr;
        JPH::uint32 index_count = 0;

        PolygonData() {}

        PolygonData(const Vertex* vtx, int size)
        {
            vertex       = new VERTEX3D[size];
            vertex_count = 0;
            for(int i = 0; i < size; ++i) {
                memcpy(&vertex[i].pos, &vtx[i].mPosition, sizeof(float) * 3);
                memcpy(&vertex[i].norm, &vtx[i].mNormal, sizeof(float) * 3);
                memcpy(&vertex[i].dif, &vtx[i].mColor, sizeof(char) * 4);
                vertex[i].u = vtx[i].mUV.x;
                vertex[i].v = vtx[i].mUV.y;

                vertex[i].spc.r = BYTE(0xFF * 0.1f);
                vertex[i].spc.g = BYTE(0xFF * 0.1f);
                vertex[i].spc.b = BYTE(0xFF * 0.1f);
                vertex[i].spc.a = BYTE(0xFF * 0.1f);

                vertex[i].su = 0;
                vertex[i].sv = 0;
            }
        }

        PolygonData(const Vertex* vtx, int vertex_size, const JPH::uint* idx, int index_size)
        {
            vertex       = new VERTEX3D[vertex_size];
            vertex_count = vertex_size;
            index        = new u16[index_size];
            index_count  = index_size;

            for(int i = 0; i < vertex_size; ++i) {
                memcpy(&vertex[i].pos, &vtx[i].mPosition, sizeof(float) * 3);
                memcpy(&vertex[i].norm, &vtx[i].mNormal, sizeof(float) * 3);
                memcpy(&vertex[i].dif, &vtx[i].mColor, sizeof(char) * 4);
                vertex[i].u = vtx[i].mUV.x;
                vertex[i].v = vtx[i].mUV.y;

                vertex[i].spc.r = BYTE(0xFF * 0.1f);
                vertex[i].spc.g = BYTE(0xFF * 0.1f);
                vertex[i].spc.b = BYTE(0xFF * 0.1f);
                vertex[i].spc.a = BYTE(0xFF * 0.1f);

                vertex[i].su = 0;
                vertex[i].sv = 0;
            }

            for(int i = 0; i < index_size; ++i) {
                index[i] = (u16)idx[i];
            }
        }

        virtual ~PolygonData()
        {
            if(vertex) {
                delete[] vertex;
                vertex = nullptr;
            }
            if(index) {
                delete[] index;
                index = nullptr;
            }
        }
    };

protected:
    JPH::Mutex mMutex;
    u32        mNextBatchID;

public:
    class BatchImpl : public JPH::RefTargetVirtual
    {
    public:
        /* JPH_OVERRIDE_NEW_DELETE */    // ワーニングが出るため展開
        JPH_INLINE void* operator new(size_t inCount) { return JPH::Allocate(inCount); }
        JPH_INLINE void  operator delete(void* inPointer) noexcept { JPH::Free(inPointer); }
        JPH_INLINE void* operator new[](size_t inCount) { return JPH::Allocate(inCount); }
        JPH_INLINE void  operator delete[](void* inPointer) noexcept { JPH::Free(inPointer); }
        JPH_INLINE void* operator new(size_t inCount, std::align_val_t inAlignment) { return JPH::AlignedAllocate(inCount, static_cast<size_t>(inAlignment)); }
        JPH_INLINE void  operator delete(void* inPointer, std::align_val_t inAlignment) noexcept
        {
            inAlignment;    // no use...
            JPH::AlignedFree(inPointer);
        }

        JPH_INLINE void* operator new[](size_t inCount, std::align_val_t inAlignment)
        {
            return JPH::AlignedAllocate(inCount, static_cast<size_t>(inAlignment));
        }

        JPH_INLINE void operator delete[](void* inPointer, std::align_val_t inAlignment) noexcept
        {
            inAlignment;    // no use...
            JPH::AlignedFree(inPointer);
        }

        BatchImpl() {}

        BatchImpl(const Triangle* inTriangles, int inTriangleCount)
        {
            if(inTriangles == nullptr || inTriangleCount <= 0)
                return;

            polygons = new PolygonData((const Vertex*)inTriangles, inTriangleCount * 3);
            //memcpy( polygons->vertex, inTriangles, sizeof( Triangle ) * inTriangleCount );
        }

        BatchImpl(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount)
        {
            if(inVertices == nullptr || inIndices == nullptr || inVertexCount <= 0 || inIndexCount <= 0)
                return;

            polygons = new PolygonData(inVertices, inVertexCount, inIndices, inIndexCount);
            //memcpy( polygons->vertex, inVertices, sizeof( Vertex ) * inVertexCount );
            //memcpy( polygons->index, inIndices, sizeof( JPH::uint ) * inIndexCount );
        }

        virtual ~BatchImpl()
        {
            if(polygons != nullptr) {
                delete polygons;
                polygons = nullptr;
            }
        }

        virtual void AddRef() override { ++mRefCount; }

        virtual void Release() override
        {
            if(--mRefCount == 0)
                delete this;
        }

        PolygonData* GetPolygons() { return polygons; }

    protected:
        JPH::atomic<JPH::uint32> mRefCount = 0;

        PolygonData* polygons = nullptr;
        JPH::uint    index    = 0;
    };

    BPDebugRenderer() { Initialize(); }

    virtual ~BPDebugRenderer() {}

    void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
    {
        DrawLine3D({inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()}, {inTo.GetX(), inTo.GetY(), inTo.GetZ()}, inColor.GetUInt32());
    }

    void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor) override
    {
        DrawLine3D({inV1.GetX(), inV1.GetY(), inV1.GetZ()}, {inV2.GetX(), inV2.GetY(), inV2.GetZ()}, inColor.GetUInt32());
        DrawLine3D({inV2.GetX(), inV2.GetY(), inV2.GetZ()}, {inV3.GetX(), inV3.GetY(), inV3.GetZ()}, inColor.GetUInt32());
        DrawLine3D({inV3.GetX(), inV3.GetY(), inV3.GetZ()}, {inV1.GetX(), inV1.GetY(), inV1.GetZ()}, inColor.GetUInt32());
    }

    Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override
    {
        if(inTriangles == nullptr || inTriangleCount == 0)
            return nullptr;

        JPH::lock_guard lock(mMutex);

        // 解放は受取先で行っている
        return new BatchImpl(inTriangles, inTriangleCount);
    }

    Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override
    {
        if(inVertices == nullptr || inVertexCount <= 0 || inIndices == nullptr || inIndexCount <= 0)
            return nullptr;

        JPH::lock_guard lock(mMutex);

        // 解放は受取先で行っている
        return new BatchImpl(inVertices, inVertexCount, inIndices, inIndexCount);
    }

    void DrawGeometry(JPH::RMat44Arg     inModelMatrix,
                      const JPH::AABox&  inWorldSpaceBounds,
                      float              inLODScaleSq,
                      JPH::ColorArg      inModelColor,
                      const GeometryRef& inGeometry,
                      ECullMode          inCullMode   = ECullMode::CullBackFace,
                      ECastShadow        inCastShadow = ECastShadow::On,
                      EDrawMode          inDrawMode   = EDrawMode::Solid) override
    {
        inCastShadow;          // no use...
        inLODScaleSq;          // no use...
        inWorldSpaceBounds;    // no use...

        for(auto& p : inGeometry->mLODs) {
            BatchImpl*   bat = (BatchImpl*)p.mTriangleBatch.GetPtr();
            PolygonData* dat = bat->GetPolygons();

            if(dat->index != nullptr) {
                auto mat = DxLib::GetCameraViewMatrix();

                auto x = inModelMatrix.GetAxisX();
                auto y = inModelMatrix.GetAxisY();
                auto z = inModelMatrix.GetAxisZ();
                auto w = inModelMatrix.GetTranslation();

                int type      = inDrawMode == EDrawMode::Wireframe ? DX_PRIMTYPE_LINELIST : DX_PRIMTYPE_TRIANGLELIST;
                int cull_type = DX_CULLING_NONE;
                if(inCullMode == ECullMode::CullBackFace) {
                    cull_type = DX_CULLING_LEFT;
                }
                else if(inCullMode == ECullMode::CullFrontFace) {
                    cull_type = DX_CULLING_RIGHT;
                }

                DxLib::SetUseZBuffer3D(TRUE);
                MATRIX mm = matrix(*(float4*)&x, *(float4*)&y, *(float4*)&z, *(float4*)&w);
                //mm		  = MScale( mm, inLODScaleSq );
                mm        = MMult(mm, mat);
                auto cull = DxLib::GetUseBackCulling();
                DxLib::SetUseBackCulling(cull_type);
                DxLib::SetUseLighting(FALSE);
                DxLib::SetCameraViewMatrix(mm);

                SetDrawAddColor(-(255 - inModelColor.b), -(255 - inModelColor.g), -(255 - inModelColor.r));

                DxLib::DrawPrimitiveIndexed3D(dat->vertex, dat->vertex_count, dat->index, dat->index_count, type, DX_NONE_GRAPH, FALSE);
                DxLib::SetCameraViewMatrix(mat);
                DxLib::SetUseLighting(TRUE);
                DxLib::SetUseBackCulling(cull);

                SetDrawAddColor(0, 0, 0);
            }
        }
    }

    /// Draw text
    void DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f)
    {
        // todo 実装
        inPosition;
        inString;
        inColor;
        inHeight;
    }

private:
};
#endif
}    // namespace

BPDebugRenderer* debug_renderer  = nullptr;
int              debug_ref_count = 0;

u16 ComponentPhysics::hit_status_[CollisionTypeSize] = {};

ComponentPhysics::~ComponentPhysics()
{
#if 0
	if( constraint_ )
	{
		physics::Engine::physicsSystem()->RemoveConstraint( constraint_ );
		constraint_ = nullptr;
	}
#endif
}

void ComponentPhysics::Init()
{
    // 一旦すべてに当たるように設定する
    if(!initialized) {
        for(int i = 0; i < CollisionTypeSize; ++i) hit_status_[i] = 0xffff;

        if(debug_renderer == nullptr) {
            debug_renderer = new BPDebugRenderer();
        }
    }
    debug_ref_count++;
    initialized = true;

    Super::Init();

    //shape_data.box_size = { 1, 1, 1 };
    //CreateBox( { 1, 1, 1 } );
    //SetCollisionType( CollisionType::ETC );

    auto sys = physics::Engine::physicsSystem();
    sys->SetContactListener(&_contactlistener);

    //_layerSettings();
#if 0
	bool active = GetPhysicsStatus( PhysicsBit::Actived );

	if( active )
	{
		body_->setGravityFactor( gravity_factor_ );
		physics::Engine::bodyInterface()->ActivateBody( GetBodyID( body_ ) );
	}
	else
	{
		body_->setGravityFactor( 0 );
		physics::Engine::bodyInterface()->DeactivateBody( GetBodyID( body_ ) );
	}

	body_->setFriction( 100.0f );
	body_->setRestitution( 0.0f );
#endif
    // モデルの姿勢が完了した後実行したい
    //Scene::GetCurrentScene()->SetPriority( shared_from_this(), ProcTiming::PostUpdate, (Priority)((int)Priority::HIGHEST+100) );
}

void ComponentPhysics::PrePhysics()
{
    Super::PrePhysics();

    // 一フレーム前の情報を確保しておく
    physics_transform_old_ = physics_transform_;
    world_old_             = GetWorldMatrix();    //body_->worldMatrix();
}

void ComponentPhysics::PostPhysics()
{
    Super::PostPhysics();

    if(!body_)
        return;

    // 回転を整える
    matrix mat = matrix(body_->worldMatrix());
#if 0
	// y軸固定
	{
		float3 vz = mat.axisZ();
		vz.y	  = 0;
		auto m = HelperLib::Math::CreateMatrixByFrontVector( vz, { 0, 1, 0 }, false );
		mat[ 0 ] = m[ 0 ];
		mat[ 1 ] = m[ 1 ];
		mat[ 2 ] = m[ 2 ];
		quaternion q( (float3x3)m );
		body_->setRotation( q );
	}
#endif

    // 物理位置からphysics分を差し引いたものをObject位置とする
    //auto mat = mul( inverse( physics_transform_ ), body_->worldMatrix() );
    mat = mul(inverse(physics_transform_), mat);
    GetOwner()->SetMatrix(mat);

    ComponentPtrVec cmps;
    if(GetOwner()->GetStatus(::Object::StatusBit::OnHitAllComponent))
        cmps = GetOwner()->GetComponents<Component>();

    std::scoped_lock lock(mutex_infos_);
    for(int i = 0; i < infos_.size(); i++) {
        auto info = infos_[i];
        OnHit(info);
        if(OnHitFunc)
            OnHitFunc(info);
        GetOwner()->OnHitPhysics(info);
        if(GetOwner()->OnHitPhysicsFunc)
            GetOwner()->OnHitPhysicsFunc(info);

        for(auto cmp : cmps) {
            if(std::dynamic_pointer_cast<ComponentCollision>(cmp))
                continue;
            if(std::dynamic_pointer_cast<ComponentPhysics>(cmp))
                continue;

            Component::HitInfoPhysics hit_info{
                .hit_ = true,
            };
            hit_info.collision_     = info.collision_;
            hit_info.hit_collision_ = info.hit_collision_;
            hit_info.hit_position_  = info.hit_position_;
            cmp->OnHitComponent(hit_info);
            if(cmp->OnHitComponentPhysicsFunc)
                cmp->OnHitComponentPhysicsFunc(hit_info);
        }
    }
    infos_.clear();
    infos_count_ = 0;
}

void ComponentPhysics::Draw()
{
    Super::Draw();

    // Debugコリジョン表示
    //エディターモードや、ShowInGameフラグがない場合は、表示しない
    if((!IsShowDebug() || !IsShowGrid()))
        return;

    if(!body_)
        return;

    auto pos = GetPhysicsMatrix().translate();

    auto ref   = physics::Engine::bodyInterface()->GetShape(GetBodyID(body_));
    int  shape = 0;

    if(ref)
        shape = (int)ref->GetSubType();

    matrix      _mat = GetPhysicsMatrix();
    JPH::RMat44 mat  = JPH::RMat44(*(JPH::Vec4*)&_mat[0], *(JPH::Vec4*)&_mat[1], *(JPH::Vec4*)&_mat[2], *(JPH::Vec4*)&_mat[3]);

    DxLib::SetLightEnable(FALSE);
    DxLib::SetUseLighting(FALSE);
    switch(shape) {
    // Convex shapes
    case(int)JPH::EShapeSubType::Box:
        //DrawBox3D( GetPhysicsMatrix(), shape_data.box_size );
        if(debug_renderer)
            debug_renderer->DrawWireBox(mat,
                                        JPH::AABox(JPH::Vec3{-shape_data.box_size.x, -shape_data.box_size.y, -shape_data.box_size.z},
                                                   JPH::Vec3{shape_data.box_size.x, shape_data.box_size.y, shape_data.box_size.z}),
                                        JPH::Color(physics_color));
        break;

    case(int)JPH::EShapeSubType::Sphere:
        pos += shape_data.sphere_center;
        //DrawSphere3D( cast( pos ), shape_data.sphere_radius, 8, physics_color, GetColor( 0, 0, 0 ), FALSE );
        if(debug_renderer)
            debug_renderer->DrawWireSphere({pos.x, pos.y, pos.z}, shape_data.sphere_radius, JPH::Color(physics_color));
        break;

    case(int)JPH::EShapeSubType::Capsule:
        if(debug_renderer)
            debug_renderer->DrawCapsule(mat,
                                        shape_data.cylinder_half_height,
                                        shape_data.cylinder_radius,
                                        JPH::Color(physics_color),
                                        JPH::DebugRenderer::ECastShadow::Off,
                                        JPH::DebugRenderer::EDrawMode::Wireframe);
        break;

    case(int)JPH::EShapeSubType::Cylinder:
        if(debug_renderer)
            debug_renderer->DrawCylinder(mat,
                                         shape_data.cylinder_half_height,
                                         shape_data.cylinder_radius,
                                         JPH::Color(physics_color),
                                         JPH::DebugRenderer::ECastShadow::Off,
                                         JPH::DebugRenderer::EDrawMode::Wireframe);
        break;

    case(int)JPH::EShapeSubType::Mesh:
        // mesh
#if 1
        if(ref_model_ < 0) {
            auto mdl   = GetOwner()->GetComponent<ComponentModel>();
            ref_model_ = mdl->GetModel();
        }
        {
            MV1RefreshReferenceMesh(ref_model_, -1, TRUE);
            //MV1RefreshCollInfo( ref_model_ );
            ref_poly_ = MV1GetReferenceMesh(ref_model_, -1, TRUE);

            SetUseLighting(FALSE);
            SetLightEnable(FALSE);

            // ポリゴンの数だけ繰り返し
            for(int i = 0; i < ref_poly_.PolygonNum; i++) {
                float3 p0 = cast(ref_poly_.Vertexs[ref_poly_.Polygons[i].VIndex[0]].Position);
                float3 p1 = cast(ref_poly_.Vertexs[ref_poly_.Polygons[i].VIndex[1]].Position);
                float3 p2 = cast(ref_poly_.Vertexs[ref_poly_.Polygons[i].VIndex[2]].Position);

                // ポリゴンを形成する三頂点を使用してワイヤーフレームを描画する
                DrawLine3D(cast(p0), cast(p1), physics_color);
                DrawLine3D(cast(p1), cast(p2), physics_color);
                DrawLine3D(cast(p2), cast(p0), physics_color);
            }

            SetLightEnable(TRUE);
            SetUseLighting(TRUE);
        }
#endif
        break;
    }
    DxLib::SetLightEnable(TRUE);
    DxLib::SetUseLighting(TRUE);
}

void ComponentPhysics::Exit()
{
    Super::Exit();

    debug_ref_count--;

    if(constraint_) {
        physics::Engine::physicsSystem()->RemoveConstraint(constraint_);
        constraint_ = nullptr;
    }
    if(body_)
        collisions_.erase(GetBodyID(body_));

    body_ = nullptr;

    if(debug_ref_count <= 0) {
        if(debug_renderer) {
            auto sys = physics::Engine::physicsSystem();
            sys->SetContactListener(nullptr);

            delete debug_renderer;
            debug_renderer = nullptr;
            initialized    = false;
        }
    }
}

void ComponentPhysics::GUI()
{
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());

    if(ImGui::TreeNode(u8"Physics")) {
        ImGui::Separator();

        if(ImGui::Button(u8"削除")) {
            GetOwner()->RemoveComponent(shared_from_this());
        }

        // 形状
        {
            int shape = -1;
            if(body_) {
                auto ref = physics::Engine::bodyInterface()->GetShape(GetBodyID(body_));
                if(ref)
                    shape = (int)ref->GetSubType();
            }

            auto shape_old = shape;

            switch(shape) {
            default:
                shape = 0;
                break;
            // Convex shapes
            case(int)JPH::EShapeSubType::Box:
                shape = 1;
                break;
            case(int)JPH::EShapeSubType::Sphere:
                shape = 2;
                break;
            case(int)JPH::EShapeSubType::Capsule:
                shape = 3;
                break;
            case(int)JPH::EShapeSubType::Cylinder:
                shape = 4;
                break;
            case(int)JPH::EShapeSubType::Mesh:
                shape = 5;
                break;
            }

            auto shape_data_old = shape_data;

            auto owner = GetOwner();

            if(ImGui::Combo(u8"型(Shape)", &shape, items, 6)) {
                // タイプが異なる場合は値もリセットする
                if(shape != shape_old) {
                    switch(shape) {
                    case 0:
                        // to NONE
                        body_ = nullptr;
                        if(constraint_)
                            physics::Engine::physicsSystem()->RemoveConstraint(constraint_);

                        constraint_ = nullptr;
                        break;
                    case 1:
                        // to BOX
                        shape_data.box_size = {1, 1, 1};
                        CreateBox(shape_data.box_size, density_);
                        break;
                    case 2:
                        // to Sphere
                        shape_data.sphere_center = {0, 0, 0};
                        shape_data.sphere_radius = 1;
                        CreateSphere(shape_data.sphere_center, shape_data.sphere_radius, density_);
                        break;
                    case 3:
                        // to Capsule
                        shape_data.cylinder_half_height = 1;
                        shape_data.cylinder_radius      = 1;
                        CreateCapsule(shape_data.cylinder_half_height, shape_data.cylinder_radius, density_);
                        break;
                    case 4:
                        // to Cylinder
                        shape_data.cylinder_half_height = 1;
                        shape_data.cylinder_radius      = 1;
                        CreateCylinder(shape_data.cylinder_half_height, shape_data.cylinder_radius, density_);
                        break;
                    case 5:
                        // to Model
                        if(auto mdl = owner->GetComponent<ComponentModel>()) {
                            CreateMesh(mdl, 1.0f);
                        }
                        break;
                    }
                }
            }

            int col_type = (int)log2((int)type_);
            if(ImGui::Combo(u8"タイプ(type)", &col_type, col_types, 16)) {
                //SetCollisionType( (CollisionType)( 1 << col_type ) );
                type_ = (CollisionType)(1 << col_type);
            }
            if(ImGui::TreeNode(u8"Hit")) {
                for(int i = 0; i < CollisionTypeSize; ++i) {
                    if(ImGui::TreeNode(col_types[i])) {
                        int  hit_status_int = hit_status_[i];
                        int* bit            = &hit_status_int;
                        for(int j = 0; j < CollisionTypeSize; ++j) {
                            if(ImGui::CheckboxFlags(col_types[j], bit, 1 << j)) {
                                hit_status_[i] = (u16)hit_status_int;
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }

            {
#if 0
				// 値が変わっているか確認し、違う場合は作り直し
				bool change = ( type == 1 &&
								( shape_data.box_size.x != shape_data_old.box_size.x ||	  //
								  shape_data.box_size.y != shape_data_old.box_size.y ||	  //
								  shape_data.box_size.z != shape_data_old.box_size.z ) ) ||
					( type == 2 &&
					  ( shape_data.sphere_center.x != shape_data_old.sphere_center.x ||	  //
						shape_data.sphere_center.y != shape_data_old.sphere_center.y ||	  //
						shape_data.sphere_center.z != shape_data_old.sphere_center.z ||
						shape_data.sphere_radius != shape_data_old.sphere_radius ) );
#endif
                {
                    ImGui::DragFloat(u8"物理利用率", &physics_real_, 0.01f, 0.0f, 1.0f, "%.2f");
                }

                {
                    bool value_change  = false;
                    value_change      |= ImGui::DragFloat(u8"質量(density)", (float*)&density_);

                    if(density_ <= 0.0f)
                        density_ = 0.01f;

                    switch(shape) {
                    case 0:
                        // to NONE
                        break;

                    case 1:
                        // to BOX
                        value_change |= ImGui::DragFloat3(u8"大きさ(XYZ size)", (float*)&shape_data.box_size, 0.5f);

                        if(value_change)
                            CreateBox(shape_data.box_size, density_);
                        break;

                    case 2:
                        // to Sphere
                        value_change |= (/* ImGui::DragFloat3( "sphere center", (float*)&shape_data.sphere_center, 0.1f ) || */
                                         ImGui::DragFloat(u8"半径(radius)", &shape_data.sphere_radius, 0.1f, 0.1f, 100.0f, "%.01f"));
                        if(value_change)
                            CreateSphere(shape_data.sphere_center, shape_data.sphere_radius, density_);
                        break;

                    case 3:
                        // to Capsule
                        value_change |= (ImGui::DragFloat(u8"高さ(half height)", &shape_data.cylinder_half_height, 0.1f, 0.1f, 100.0f, "%.01f") ||
                                         ImGui::DragFloat(u8"半径(radius)", &shape_data.cylinder_radius, 0.1f, 0.1f, 100.0f, "%.01f"));
                        if(value_change)
                            CreateCapsule(shape_data.cylinder_half_height, shape_data.cylinder_radius, density_);
                        break;

                    case 4:
                        // to Cylinder
                        value_change |= (ImGui::DragFloat(u8"高さ(half height)", &shape_data.cylinder_half_height, 0.1f, 0.1f, 100.0f, "%.01f") ||
                                         ImGui::DragFloat(u8"半径(radius)", &shape_data.cylinder_radius, 0.1f, 0.1f, 100.0f, "%.01f"));
                        if(value_change)
                            CreateCylinder(shape_data.cylinder_half_height, shape_data.cylinder_radius, density_);
                        break;
#if 0
					case 5:
						// to Model
						if( auto mdl = owner->GetComponent<ComponentModel>() )
						{
							CreateMesh( mdl, 1.0f );
						}
						break;
#endif
                    }
                    if(value_change) {
                        if(GetPhysicsStatus(PhysicsBit::Actived)) {
                            body_->setGravityFactor(gravity_factor_);
                            physics::Engine::bodyInterface()->ActivateBody(GetBodyID(body_));
                        }
                        else {
                            body_->setGravityFactor(0);
                            physics::Engine::bodyInterface()->DeactivateBody(GetBodyID(body_));
                        }
                    }
                }
            }

            // モデルコンポーネント表示
            {
                // モデル姿勢
                if(ImGui::TreeNode(UNIQUE_TEXT(u8"物理姿勢(matrix)"))) {
                    ImGui::DragFloat4(UNIQUE_TEXT(u8"Ｘ軸"), physics_transform_.f32_128_0, 0.01f, -10000.0f, 10000.0f, "%.2f");
                    ImGui::DragFloat4(UNIQUE_TEXT(u8"Ｙ軸"), physics_transform_.f32_128_1, 0.01f, -10000.0f, 10000.0f, "%.2f");
                    ImGui::DragFloat4(UNIQUE_TEXT(u8"Ｚ軸"), physics_transform_.f32_128_2, 0.01f, -10000.0f, 10000.0f, "%.2f");
                    ImGui::DragFloat4(UNIQUE_TEXT(u8"座標"), physics_transform_.f32_128_3, 0.01f, -10000.0f, 10000.0f, "%.2f");
                    ImGui::Separator();
                    ImGui::TreePop();
                }

                // 姿勢を TRSで変更できるように設定
                float* mat = physics_transform_.f32_128_0;
                float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
                DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
                ImGui::DragFloat3(UNIQUE_TEXT(u8"座標(T)"), matrixTranslation, 0.01f, -100000.00f, 100000.0f, "%.2f");
                ImGui::DragFloat3(UNIQUE_TEXT(u8"回転(R)"), matrixRotation, 0.1f, -360.0f, 360.0f, "%.2f");
                ImGui::DragFloat3(UNIQUE_TEXT(u8"スケール(S)"), matrixScale, 0.01f, 0.00f, 1000.0f, "%.2f");
                RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

                //body_->setPosition( physics_transform_.translate() );
                //body_->setRotation( quaternion( float3x3( physics_transform_ ) ) );
            }

            // Physics有効
            if(body_) {
                float3 linear = body_->linearVelocity();
                if(ImGui::DragFloat3(UNIQUE_TEXT(u8"移動方向(LinearVelocity)"), (float*)&linear, 0.1f))
                    body_->setLinearVelocity(linear);

                if(ImGui::Button(u8"動作リセット")) {
                    body_->setAngularVelocity({0, 0, 0});
                    body_->setLinearVelocity({0, -0.01f, 0});
                    body_->setPosition({0, 20, 0});
                    body_->setRotation({0, 0, 0, 1}, false);
                    owner->SetTranslate({0, 20, 0});
                }

                auto               ref       = physics::Engine::bodyInterface()->GetShape(GetBodyID(body_));
                JPH::EShapeSubType body_type = JPH::EShapeSubType::User1;
                if(ref)
                    body_type = ref->GetSubType();

                if(body_type == JPH::EShapeSubType::Mesh)
                    ImGui::BeginDisabled();

                if(ImGui::DragFloat(u8"重力係数(gravityFactor)", &gravity_factor_, 0.05f, 0.0f, 5.0f))
                    body_->setGravityFactor(gravity_factor_);

                bool active = GetPhysicsStatus(PhysicsBit::Actived);
                if(ImGui::Checkbox(u8"重力有効(Active)", &active)) {
                    if(active) {
                        body_->setGravityFactor(gravity_factor_);
                        physics::Engine::bodyInterface()->ActivateBody(GetBodyID(body_));
                    }
                    else {
                        body_->setGravityFactor(0);
                        physics::Engine::bodyInterface()->DeactivateBody(GetBodyID(body_));
                    }
                    SetPhysicsStatus(PhysicsBit::Actived, active);
                }

                if(body_type == JPH::EShapeSubType::Mesh)
                    ImGui::EndDisabled();

                float friction = body_->friction();
                if(ImGui::DragFloat(u8"摩擦力(Friction)", &friction, 0.05f, 0.0f, 1000.0f))
                    body_->setFriction(friction);

                float restitution = body_->restitution();
                if(ImGui::DragFloat(u8"跳返り係数(Restitution)", &restitution, 0.05f, 0.0f, 1.0f))
                    body_->setRestitution(restitution);

                u32* bit = &physics_status_.get();

                //ImGui::CheckboxFlags( u8"初期化済", &val, 1 << (int)PhysicsBit::Initialized );
                bool refresh  = ImGui::CheckboxFlags(u8"X軸回転禁止", bit, 1 << (int)PhysicsBit::LockRotX);
                refresh      |= ImGui::CheckboxFlags(u8"Y軸回転禁止", bit, 1 << (int)PhysicsBit::LockRotY);
                refresh      |= ImGui::CheckboxFlags(u8"Z軸回転禁止", bit, 1 << (int)PhysicsBit::LockRotZ);

                if(refresh) {
                    LockRotateAxis(GetPhysicsStatus(PhysicsBit::LockRotX), GetPhysicsStatus(PhysicsBit::LockRotY), GetPhysicsStatus(PhysicsBit::LockRotZ));
                }
            }
        }
        ImGui::TreePop();
    }
    ImGui::End();
}

void ComponentPhysics::MoveTo(const matrix& mat)
{
    if(auto body = GetRigidBody()) {
        body->moveKinematic(mat.translate(), quaternion(float3x3(mat)), GetDeltaTime());
    }
}

void ComponentPhysics::MoveToWorldMatrix()
{
    MoveTo(GetWorldMatrix());
}

void ComponentPhysics::LockRotateAxis(bool x, bool y, bool z)
{
    if(constraint_)
        physics::Engine::physicsSystem()->RemoveConstraint(constraint_);

    sSettings         = new JPH::SixDOFConstraintSettings();
    sSettings->mSpace = JPH::EConstraintSpace::LocalToBodyCOM;
    if(x)
        sSettings->MakeFixedAxis(JPH::SixDOFConstraintSettings::EAxis::RotationX);

    if(y)
        sSettings->MakeFixedAxis(JPH::SixDOFConstraintSettings::EAxis::RotationY);

    if(z)
        sSettings->MakeFixedAxis(JPH::SixDOFConstraintSettings::EAxis::RotationZ);

    constraint_ = (JPH::SixDOFConstraint*)physics::Engine::bodyInterface()->CreateConstraint(sSettings, JPH::Body::sFixedToWorld.GetID(), GetBodyID(body_));
    physics::Engine::physicsSystem()->AddConstraint(constraint_);
}

bool ComponentPhysics::CreateBox(float3 size, float density)
{
    size.x = std::max((float)size.x, 0.1f);
    size.y = std::max((float)size.y, 0.1f);
    size.z = std::max((float)size.z, 0.1f);

    body_ = physics::createRigidBody(shape::Box{size}, physics::ObjectLayers::MOVING, physics::MotionType::Dynamic, density);
    collisions_.insert(std::pair(GetBodyID(body_), SharedThis()));
    //body_->setData( (intptr_t)this );
    body_->setGravityFactor(gravity_factor_);
    density_ = density;

    SetPhysicsStatus(ComponentPhysics::PhysicsBit::Static, false);
    SetPhysicsMatrix(GetWorldMatrix());

    shape_data.box_size = size;

    LockRotateAxis(GetPhysicsStatus(PhysicsBit::LockRotX), GetPhysicsStatus(PhysicsBit::LockRotY), GetPhysicsStatus(PhysicsBit::LockRotZ));

    return true;
}

bool ComponentPhysics::CreateSphere(float3 center, float radius, float density)
{
    radius = std::max(radius, 0.1f);

    body_ = physics::createRigidBody(shape::Sphere{center, radius}, physics::ObjectLayers::MOVING, physics::MotionType::Dynamic, density);
    collisions_.insert(std::pair(GetBodyID(body_), SharedThis()));
    //body_->setData( (intptr_t)this );
    body_->setGravityFactor(gravity_factor_);
    density_ = density;

    SetPhysicsStatus(ComponentPhysics::PhysicsBit::Static, false);
    SetPhysicsMatrix(GetWorldMatrix());

    //TODO ADD
    shape_data.sphere_center = center;
    shape_data.sphere_radius = radius;

    LockRotateAxis(GetPhysicsStatus(PhysicsBit::LockRotX), GetPhysicsStatus(PhysicsBit::LockRotY), GetPhysicsStatus(PhysicsBit::LockRotZ));

    return true;
}

bool ComponentPhysics::CreateCapsule(float half_height, float radius, float density)
{
    if(radius <= 0.1f)
        radius = 0.1f;

    body_ = physics::createRigidBody(shape::Capsule{half_height, radius}, physics::ObjectLayers::MOVING, physics::MotionType::Dynamic, density);
    collisions_.insert(std::pair(GetBodyID(body_), SharedThis()));
    //body_->setData( (intptr_t)this );
    body_->setGravityFactor(gravity_factor_);
    density_ = density;

    SetPhysicsStatus(ComponentPhysics::PhysicsBit::Static, false);
    SetPhysicsMatrix(GetWorldMatrix());

    //TODO ADD
    shape_data.cylinder_half_height = half_height;
    shape_data.cylinder_radius      = radius;

    LockRotateAxis(GetPhysicsStatus(PhysicsBit::LockRotX), GetPhysicsStatus(PhysicsBit::LockRotY), GetPhysicsStatus(PhysicsBit::LockRotZ));
    return true;
}

bool ComponentPhysics::CreateCylinder(float half_height, float radius, float density)
{
    if(radius <= 0.1f)
        radius = 0.1f;

    body_ = physics::createRigidBody(shape::Cylinder{half_height, radius}, physics::ObjectLayers::MOVING, physics::MotionType::Dynamic, density);
    collisions_.insert(std::pair(GetBodyID(body_), SharedThis()));
    //body_->setData( (intptr_t)this );
    body_->setGravityFactor(gravity_factor_);
    density_ = density;

    SetPhysicsStatus(ComponentPhysics::PhysicsBit::Static, false);
    SetPhysicsMatrix(GetWorldMatrix());

    //TODO ADD
    shape_data.cylinder_half_height = half_height;
    shape_data.cylinder_radius      = radius;

    LockRotateAxis(GetPhysicsStatus(PhysicsBit::LockRotX), GetPhysicsStatus(PhysicsBit::LockRotY), GetPhysicsStatus(PhysicsBit::LockRotZ));
    return true;
}

bool ComponentPhysics::CreateMesh(ComponentModelPtr model, float scale)
{
    float3 scale3  = model->GetScaleAxisXYZ();
    scale         *= ((scale3.x + scale3.y + scale3.z) / 3.0f);

    ref_model_ = model->GetModel();

    if(constraint_)
        physics::Engine::physicsSystem()->RemoveConstraint(constraint_);

    constraint_ = nullptr;

    body_ = physics::createRigidBody(shape::Mesh{model->GetModelClass(), scale}, physics::ObjectLayers::NON_MOVING);
    collisions_.insert(std::pair(GetBodyID(body_), SharedThis()));
    //body_->setData( (intptr_t)this );

    SetPhysicsStatus(ComponentPhysics::PhysicsBit::Static, true);
    SetPhysicsMatrix(GetWorldMatrix());
    return true;
}

//! @brief ワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置
const matrix ComponentPhysics::GetWorldMatrix() const
{
    return mul(physics_transform_, GetOwner()->GetWorldMatrix());
}

//! @brief 1フレーム前のワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置
const matrix ComponentPhysics::GetOldWorldMatrix() const
{
    return mul(physics_transform_old_, world_old_);
}

void ComponentPhysics::SetPhysicsMatrix(const matrix& world)
{
    if(!body_)
        return;

    body_->setPosition(world.translate());
    auto q = normalize(quaternion(float3x3(world)));
    body_->setRotation(q);
}

matrix ComponentPhysics::GetPhysicsMatrix() const
{
    return body_->worldMatrix();
}
