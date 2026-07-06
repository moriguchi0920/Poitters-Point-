//---------------------------------------------------------------------------
//! @file   Component.h
//! @brief  コンポーネント ベースクラス
//---------------------------------------------------------------------------
#pragma once

#include <System/ProcTiming.h>
#include <System/Status.h>

#include <cereal/cereal.hpp>

// ポインター宣言
USING_PTR(Component);
USING_PTR(Object);

//! Componentルートの型情報
class ComponentTypeInfo : public TypeInfo
{
public:
	ComponentTypeInfo(const char* class_name, size_t class_size, ComponentTypeInfo* parent_type = nullptr, const char* desc_name = "");
	//! ルートの型情報
	static ComponentTypeInfo component_root;

	ComponentPtr	 tmp = std::shared_ptr<class Component>((Component*)createInstance());
	ComponentWeakPtr ptr = tmp;

	virtual void* createComponentPtr(const ObjectPtr&)
	{
		return reinterpret_cast<void*>(&ptr);
	}
};

//! @note   基底クラスでSuperを宣言するとときにこのクラスが参照されます
class ComponentClass
{
public:
	static inline ComponentTypeInfo& Type = ComponentTypeInfo::component_root;
};

//===========================================================================
//! クラス用型情報 ClassComponentType<T>
//===========================================================================
template <class T>
class ClassComponentType : public ComponentTypeInfo
{
public:
	//  コンストラクタ
	//! @param  [in]    class_name  クラスの名前
	//! @param  [in]    parent_type 親クラスの型情報(親がないクラスはnullptr)
	//! @param  [in]    desc_name   説明文文字列
	ClassComponentType(const char* class_name, ComponentTypeInfo* parent_type = nullptr, const char* desc_name = "")
		: ComponentTypeInfo(class_name, sizeof(T), parent_type, desc_name)
	{
	}

	virtual void* createComponentPtr(const ObjectPtr& obj) override
	{
		std::shared_ptr<T> cmp = std::make_shared<T>();

		Component::RegisterToObject(cmp, obj);

		tmp = cmp;
		return reinterpret_cast<void*>(&tmp);
	}

	std::weak_ptr<T> tmp;
};

//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言
//! publicで BP_COMPONENT_DECL(クラス名, u8"任意の解説文字列") で宣言します
//!
//! @code
//!     class B : public A
//!     {
//!     public:
//!         BP_COMPONENT_DECL(B, u8"コンポーネント");
//! @endcode
//---------------------------------------------------------------------------
#define BP_COMPONENT_DECL(CLASS, ...)                                                                                          \
	using Super			 = ComponentClass; /*! 親クラスの型 : 上のクラス階層で定義されているClassを使うことで親クラスを定義 */ \
	using ComponentClass = CLASS;		   /*! 自クラスの型 : これ以降は親クラスではなく自クラスを指す */                      \
                                                                                                                               \
	/*! 型情報 */                                                                                                              \
	static inline ClassComponentType<CLASS> Type = ClassComponentType<CLASS>(#CLASS, &Super::Type, __VA_ARGS__);               \
                                                                                                                               \
	/*! 型情報を取得 */                                                                                                        \
	virtual const ComponentTypeInfo* typeInfo() const                                                                          \
	{                                                                                                                          \
		return &Type;                                                                                                          \
	}                                                                                                                          \
	void* createComponentPtr(const ObjectPtr& obj)                                                                             \
	{                                                                                                                          \
		return Type.createComponentPtr(obj);                                                                                   \
	}

//***************************************************************************
//***************************************************************************
// 削除予定 ここから
// ↓↓↓↓↓↓↓

//---------------------------------------------------------------------------
//! Componentクラス内ヘッダー宣言 基底クラス用
//! publicで BP_COMPONENT_BASE_TYPE(クラス名) で宣言します
//!
//! @code
//!     class A
//!     {
//!     public:
//!         BP_COMPONENT_BASE_TYPE(A)
//! @endcode
//---------------------------------------------------------------------------
#define BP_COMPONENT_BASE_TYPE(CLASS)                                                                                                                                           \
	using MyClass = CLASS; /*! 自クラスの型 */                                                                                                                                  \
                                                                                                                                                                                \
	/*! 型情報 */                                                                                                                                                               \
	[[deprecated("BP_COMPONENT_TYPE() と BP_COMPONENT_BASE_TYPE() は古い宣言です。BP_COMPONENT_DECL(クラス名, u8\"解説文\") に置換してください。")]] static ClassComponentType< \
		CLASS> Type;                                                                                                                                                            \
                                                                                                                                                                                \
	/*! 型情報を取得 */                                                                                                                                                         \
	virtual const ComponentTypeInfo* typeInfo() const                                                                                                                           \
	{                                                                                                                                                                           \
		return &Type;                                                                                                                                                           \
	}

//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言
//! publicで BP_COMPONENT_TYPE(クラス名, 親クラス名) で宣言します
//!
//! @code
//!     class B : public A
//!     {
//!     public:
//!         BP_COMPONENT_TYPE(B, A);
//! @endcode
//---------------------------------------------------------------------------
#define BP_COMPONENT_TYPE(CLASS, PARENT_CLASS)     \
	using Super = PARENT_CLASS; /* 親クラスの型 */ \
	BP_COMPONENT_BASE_TYPE(CLASS);

//---------------------------------------------------------------------------
//! クラス内cpp宣言
//! グローバル変数で BP_OBJECT_IMPL(クラス名, u8任意の名前文字列) で宣言します
//! @code
//! BP_OBJECT_IMPL(B, u8"クラスB")
//! @endcode
//---------------------------------------------------------------------------
#define BP_COMPONENT_BASE_IMPL(CLASS, DESC_NAME) \
	/*! 型情報の実体 */                          \
	ClassComponentType<CLASS> CLASS::Type(#CLASS, sizeof(CLASS), nullptr, DESC_NAME);

#define BP_COMPONENT_IMPL(CLASS, DESC_NAME) \
	/*! 型情報の実体 */                     \
	ClassComponentType<CLASS> CLASS::Type(#CLASS, &Super::Type, DESC_NAME);

//ClassComponentType<CLASS> CLASS::Type(#CLASS, sizeof(CLASS), &Super::Type, DESC_NAME);

// ↑↑↑↑↑↑↑
// 削除予定 ここまで
//***************************************************************************
//***************************************************************************

//---------------------------------------------------------------------------
//! @brief コンポーネント
class Component : public std::enable_shared_from_this<Component>
{
	friend class ::Object;
	friend class Scene;

public:
	BP_COMPONENT_DECL(Component, u8"Componentクラス");

	//	Component( const Component& )			 = delete;
	//	Component& operator=( const Component& ) = delete;

	virtual ~Component()
	{
		for(auto& t : proc_timings_)
		{
			auto& p = t.second;
			if(p.connect_.valid())
				p.connect_.disconnect();

			p.proc_ = nullptr;
		}
		proc_timings_.clear();
	}

	Object*			GetOwner();				//!< オーナー(従属しているオブジェクト)の取得
	const Object*	GetOwner() const;		//!< オーナー(従属しているオブジェクト)の取得
	ObjectPtr		GetOwnerPtr();			//!< オーナー(従属しているオブジェクト)の取得(SharedPtr)
	const ObjectPtr GetOwnerPtr() const;	//!< オーナー(従属しているオブジェクト)の取得(SharedPtr)

	template <class T>
	std::shared_ptr<T> SetName(const std::string_view& name)
	{
		name_ = name;
		return std::dynamic_pointer_cast<T>(shared_from_this());
	}

	const std::string_view GetName() const
	{
		return name_;
	}

	virtual void Init();		  //!< 初期化
	virtual void Update();		  //!< アップデート
	virtual void LateUpdate();	  //!< 遅いアップデート
	virtual void Draw();		  //!< 描画
	virtual void LateDraw();	  //!< 遅い描画
	virtual void Exit();		  //!< 終了
	virtual void GUI();			  //!< GUI表示

	virtual void PreUpdate();	   //!< 更新前処理
	virtual void PostUpdate();	   //!< 更新後処理
	virtual void PreDraw();		   //!< 描画前処理
	virtual void PostDraw();	   //!< 描画後処理
	virtual void PrePhysics();	   //!< Physics前処理
	virtual void PostPhysics();	   //!< Physics後処理

	virtual void InitSerialize();	 //!< シリアライズでもどらないユーザー処理関数などを設定

	void SetPriority(ProcTiming timing, ProcPriority priority);

#define UNIQUE_TEXT(n) UniqueText(n).c_str()

	std::string UniqueText(const std::string_view& name, int id = 0)
	{
		auto str = std::string(name) + "##" + std::string(name) + "." + std::to_string(id) + "." + std::to_string((size_t)(this));
		return str;
	}

	//----------------------------------------------------------
	//! @name  コンポーネントオブジェクト登録メソッド
	//----------------------------------------------------------
	//@{
	static void RegisterToObject(ComponentPtr cmp, ObjectPtr obj);

	//! 処理を取得
	SlotProc& GetProc(const std::string& proc_name, ProcTiming timing)
	{
		auto itr = proc_timings_.find(proc_name);
		if(itr != proc_timings_.end())
			return itr->second;

		proc_timings_[proc_name]		 = SlotProc();
		proc_timings_[proc_name].name_	 = proc_name;
		proc_timings_[proc_name].timing_ = timing;
		return proc_timings_[proc_name];
	}

	SlotProc& SetProc(const std::string& proc_name, ProcTimingFunc func, ProcTiming timing = ProcTiming::Update, ProcPriority prio = ProcPriority::NORMAL);

	SlotProc& SetPriority(const std::string& proc_name, ProcTiming timing = ProcTiming::Update, ProcPriority prio = ProcPriority::NORMAL);

	void ResetProc(const std::string& proc_name);

	//! @brief 自分の消去
	void RemoveThisComponent();

	enum struct StatusBit : u64
	{
		Alive = 0,		 //!< 生存状態
		ChangePrio,		 //!< プライオリティの変更中
		ShowGUI,		 //!< GUI表示中
		Initialized,	 //!< 初期化終了
		NoUpdate,		 //!< Updateしない
		NoDraw,			 //!< Drawしない
		DisablePause,	 //!< ポーズ不可
		IsPause,		 //!< ポーズ中
		SameType,		 //!< 同じタイプのコンポーネント可能
		Exited,			 //!< 正しく終了が呼ばれている
		Serialized,		 //!< シリアライズ済み.
		NoSerialize,	 //!< シリアライズしない.
		Enable,			 //!< 処理する or 処理しない(Update/Draw系なにもしない)
	};

	void SetStatus(StatusBit b, bool on);	 //!< ステータスの設定
	bool GetStatus(StatusBit b);			 //!< ステータスの取得

	Component();
	virtual void Construct(ObjectPtr owner);

	//! @brief 当たった情報はコールバックで送られてくる
	//! @param hitInfo 当たった情報
	//! @details 当たった回数分ここに来ます
	struct HitInfo;
	struct HitInfoPhysics;
	virtual void OnHitComponent(const HitInfo&)
	{
	}
	virtual void OnHitComponent(const HitInfoPhysics&)
	{
	}

	std::function<void(const HitInfo& hit_info)>		OnHitComponentFunc;
	std::function<void(const HitInfoPhysics& hit_info)> OnHitComponentPhysicsFunc;

protected:
	ObjectPtr owner_ = nullptr;	   //!< オーナー
	SlotProcs proc_timings_;	   //!< 登録処理(update)

	float update_delta_time_ = 0.0f;	//!< update以外で使用できるように

	std::string name_;

private:
	Status<StatusBit> status_;		  //!< コンポーネント状態
	Status<StatusBit> status_old_;	  //!< セット前のコンポーネント状態

	// 高速化
	std::vector<std::string> dirty_component_list_;

private:
	//--------------------------------------------------------------------
	//! @name Cereal処理
	//--------------------------------------------------------------------
	//@{

	//! @brief セーブロード
	//! @param arc アーカイバ
	//! @param ver バージョン
	CEREAL_SAVELOAD(arc, ver)
	{
		arc(CEREAL_NVP(owner_),			  //< オーナー
			CEREAL_NVP(proc_timings_),	  //< プロセスタイミング
			CEREAL_NVP(status_.get())	  //< ステータス
		);
		if(ver >= 2)
			arc(CEREAL_NVP(name_));

		status_.off(StatusBit::Serialized);
	}

	//@}
};

CEREAL_CLASS_VERSION(Component, 2);
