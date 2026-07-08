//---------------------------------------------------------------------------
//! @file   TypeInfo.h
//! @brief  クラス型情報
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! 型情報基底
//===========================================================================
class TypeInfo : noncopyable, nonmovable
{
public:
    //  コンストラクタ
    //! @param  [in]    class_name  クラスの名前
    //! @param  [in]    class_size  クラスのサイズ(単位:bytes)
    //! @param  [in]    parent_type 親クラスの型情報(親がないクラスはnullptr)
    //! @param  [in]    desc_name   説明文文字列
    TypeInfo(const char* class_name, size_t class_size, TypeInfo* parent_type = nullptr, const char* desc_name = "");

    //  インスタンスを作成(クラスをnewしてポインタを返す)
    virtual void* createInstance() const;

    //  クラス名を取得
    const char* className() const;

    //  説明文字列を取得
    const char* descName() const;

    //  クラスのサイズを取得
    size_t classSize() const;

    //  親ノードを取得
    //! @return ノードへのポインタ (存在しない場合はnullptr)
    const TypeInfo* parent() const;

    //  子ノードを取得
    //! @return ノードへのポインタ (存在しない場合はnullptr)
    const TypeInfo* child() const;

    //  次の兄弟ノードを取得
    //! @return ノードへのポインタ (存在しない場合はnullptr)
    const TypeInfo* siblings() const;

    //! ルートの型情報
    static TypeInfo Root;

private:
    const char* class_name_ = nullptr;    //!< クラス名
    const char* desc_name_  = nullptr;    //!< 説明文
    size_t      class_size_ = 0;          //!< クラスのサイズ

    //----------------------------------------------------------
    //! @name   クラス階層のツリー構造
    //! この手法で生成するツリー構造は初期化順序が不同でも構築できる
    //! 但しグローバル変数領域に定義する場合のみ利用可能。
    //!
    //! @attention 変数の初期値は意図的に設定していません。グローバル変数の0クリア仕様で動作を想定しています。
    //! @attention ここにnullptr初期化を記述すると起動時初期化順序によっては上書きされてしまうため注意
    //----------------------------------------------------------
    //@{

    const TypeInfo* parent_;      //!< 親ノード
    const TypeInfo* child_;       //!< 子ノード
    const TypeInfo* siblings_;    //!< 次の兄弟ノード

    //@}
};

//! @note   基底クラスでSuperを宣言するとときにこのクラスが参照されます
class Class
{
public:
    static inline TypeInfo& Type = TypeInfo::Root;
};

//===========================================================================
//! クラス用型情報 ClassTypeInfo<T>
//===========================================================================
template <class T>
class ClassTypeInfo : public TypeInfo
{
public:
    //  コンストラクタ
    //! @param  [in]    class_name  クラスの名前
    //! @param  [in]    parent_type 親クラスの型情報(親がないクラスはnullptr)
    //! @param  [in]    desc_name   説明文文字列
    ClassTypeInfo(const char* class_name, TypeInfo* parent_type = nullptr, const char* desc_name = "")
        : TypeInfo(class_name, sizeof(T), parent_type, desc_name)
    {
    }

    // インスタンスを作成(クラスをnewしてポインタを返す)
    virtual void* createInstance() const override { return T::createInstance(); }

private:
};

//===========================================================================
//! @name インスタンスを作成します
//! @note クラスをnewしてポインタを返します
//===========================================================================
//!@{

//! インスタンス作成クラス
template <typename T, bool is_abstract = std::is_abstract<T>::value>
class CreateInstance
{
public:
    void* operator()() { return new T(); }
};

//! 抽象クラスの場合はnewできないためnullptrを返す特殊化
template <typename T>
class CreateInstance<T, true>
{
public:
    void* operator()() { return nullptr; }
};

//@}
//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言
//! publicで BP_CLASS_TYPE(クラス名, u8"任意の解説文字列") で宣言します
//!
//! @code
//!     class B : public A
//!     {
//!     public:
//!         BP_CLASS_TYPE(B, u8"オブジェクト");
//! @endcode
//---------------------------------------------------------------------------
#define BP_CLASS_DECL(CLASS, ...)                                                                            \
    using Super = Class; /*! 親クラスの型 : 上のクラス階層で定義されているClassを使うことで親クラスを定義 */ \
    using Class = CLASS; /*! 自クラスの型 : これ以降は親クラスではなく自クラスを指す */                      \
                                                                                                             \
    /*! 型情報 */                                                                                            \
    static inline ClassTypeInfo<CLASS> Type = ClassTypeInfo<CLASS>(#CLASS, &Super::Type, __VA_ARGS__);       \
                                                                                                             \
    /*! 型情報を取得 */                                                                                      \
    virtual TypeInfo* typeInfo() const                                                                       \
    {                                                                                                        \
        return &Type;                                                                                        \
    }                                                                                                        \
                                                                                                             \
    /*! インスタンスを作成(クラスをnewしてポインタを返す)*/                                                  \
    static void* createInstance()                                                                            \
    {                                                                                                        \
        return CreateInstance<CLASS>()();                                                                    \
    }

//***************************************************************************
//***************************************************************************
// 削除予定 ここから
// ↓↓↓↓↓↓↓

//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言 基底クラス用
//! publicで BP_BASE_TYPE(クラス名) で宣言します
//!
//! @code
//!     class A
//!     {
//!     public:
//!         BP_BASE_TYPE(A)
//! @endcode
//---------------------------------------------------------------------------
#define BP_BASE_TYPE(CLASS)                                                                                                                                   \
    using MyClass = CLASS; /*! 自クラスの型 */                                                                                                                \
                                                                                                                                                              \
    /*! 型情報 */                                                                                                                                             \
    [[deprecated(                                                                                                                                             \
        "BP_BASE_TYPE() と BP_CLASS_TYPE() は廃止されました。BP_CLASS_DECL(クラス名, u8\"解説文\") に置換してください。")]] static ClassTypeInfo<CLASS> Type; \
                                                                                                                                                              \
    /*! 型情報を取得 */                                                                                                                                       \
    virtual const TypeInfo* typeInfo() const                                                                                                                  \
    {                                                                                                                                                         \
        return &Type;                                                                                                                                         \
    }                                                                                                                                                         \
                                                                                                                                                              \
    /*! インスタンスを作成(クラスをnewしてポインタを返す)*/                                                                                                   \
    static void* createInstance();

//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言
//! publicで BP_BASE_TYPE(クラス名, 親クラス名) で宣言します
//!
//! @code
//!     class B : public A
//!     {
//!     public:
//!         BP_CLASS_TYPE(B, A);
//! @endcode
//---------------------------------------------------------------------------
#define BP_CLASS_TYPE(CLASS, PARENT_CLASS)         \
    using Super = PARENT_CLASS; /* 親クラスの型 */ \
    BP_BASE_TYPE(CLASS);

//---------------------------------------------------------------------------
//! クラス内cpp宣言
//! グローバル変数で BP_CLASS_IMPL(クラス名, u8任意の名前文字列) で宣言します
//! @code
//! BP_CLASS_IMPL(B, u8"クラスB")
//! @endcode
//---------------------------------------------------------------------------
#define BP_CLASS_IMPL(CLASS, DESC_NAME)                                                                                                                                                                                                                             \
    /*! 型情報の実体 */                                                                                                                                                                                                                                             \
    namespace {                                                                                                                                                                                                                                                     \
    [[deprecated(                                                                                                                                                                                                                                                   \
        "BP_CLASS_IMPL() は廃止されました。cppに定義する必要がなくなりましたので削除してください。\n    代わりにヘッダーのBP_CLASS_TYPE(クラス名, 親クラス名)を BP_CLASS_DECL(クラス名, u8\"解説文\") に置換してください。親クラス指定も不要になりました。")]] void \
    BP_BASE_IMPL_deprecated()                                                                                                                                                                                                                                       \
    {                                                                                                                                                                                                                                                               \
    }                                                                                                                                                                                                                                                               \
    }                                                                                                                                                                                                                                                               \
                                                                                                                                                                                                                                                                    \
    ClassTypeInfo<CLASS> CLASS::Type(#CLASS, &Super::Type, DESC_NAME);                                                                                                                                                                                              \
    void*                CLASS::createInstance()                                                                                                                                                                                                                    \
    {                                                                                                                                                                                                                                                               \
        BP_BASE_IMPL_deprecated();                                                                                                                                                                                                                                  \
        return new CLASS();                                                                                                                                                                                                                                         \
    }

//---------------------------------------------------------------------------
//! クラス内cpp宣言 基底クラス用
//! グローバル変数で BP_BASE_IMPL(クラス名, u8任意の名前文字列) で宣言します
//! @code
//! BP_BASE_IMPL(A, u8"基底クラス")
//! @endcode
//---------------------------------------------------------------------------
#define BP_BASE_IMPL(CLASS, DESC_NAME)                                                                                                                                                                                                                \
    /*! 型情報の実体 */                                                                                                                                                                                                                               \
    ClassTypeInfo<CLASS> CLASS::Type(#CLASS, nullptr, DESC_NAME);                                                                                                                                                                                     \
    namespace {                                                                                                                                                                                                                                       \
    [[deprecated(                                                                                                                                                                                                                                     \
        "BP_BASE_IMPL() は廃止されました。cppに定義する必要がなくなりましたので削除してください。\n    代わりにヘッダーのBP_BASE_TYPE(クラス名)を BP_CLASS_DECL(クラス名, u8\"解説文\") に置換してください。親クラス指定も不要になりました。")]] void \
    BP_BASE_IMPL_deprecated()                                                                                                                                                                                                                         \
    {                                                                                                                                                                                                                                                 \
    }                                                                                                                                                                                                                                                 \
    }                                                                                                                                                                                                                                                 \
                                                                                                                                                                                                                                                      \
    void* CLASS::createInstance()                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                 \
        BP_BASE_IMPL_deprecated();                                                                                                                                                                                                                    \
        return new CLASS();                                                                                                                                                                                                                           \
    }

// 抽象クラス用カスタム
#define BP_BASE_IMPL_ABSOLUTE(CLASS, DESC_NAME)                                                                                                                                                                                                                \
    /*! 型情報の実体 */                                                                                                                                                                                                                                        \
    ClassTypeInfo<CLASS> CLASS::Type(#CLASS, nullptr, DESC_NAME);                                                                                                                                                                                              \
    namespace {                                                                                                                                                                                                                                                \
    [[deprecated(                                                                                                                                                                                                                                              \
        "BP_BASE_IMPL_ABSOLUTE() は廃止されました。cppに定義する必要がなくなりましたので削除してください。\n    代わりにヘッダーのBP_BASE_TYPE(クラス名)を BP_CLASS_DECL(クラス名, u8\"解説文\") に置換してください。親クラス指定も不要になりました。")]] void \
    BP_BASE_IMPL_ABSOLUTE_deprecated()                                                                                                                                                                                                                         \
    {                                                                                                                                                                                                                                                          \
    }                                                                                                                                                                                                                                                          \
    }                                                                                                                                                                                                                                                          \
    void* CLASS::createInstance()                                                                                                                                                                                                                              \
    {                                                                                                                                                                                                                                                          \
        BP_BASE_IMPL_ABSOLUTE_deprecated();                                                                                                                                                                                                                    \
        return nullptr;                                                                                                                                                                                                                                        \
    }

// ↑↑↑↑↑↑↑
// 削除予定 ここまで
//***************************************************************************
//***************************************************************************

//===========================================================================
//! @name   ユーティリティー
//===========================================================================
//@{

//! 名前を指定して指定基底クラス型でnewする
//! @param  [in]    class_name  クラス名
//! @param  [in]    base_type   基底クラスの型情報
//! @return newされたクラスオブジェクト(失敗の場合はnullptr)
[[nodiscard]] void* CreateInstanceFromName(std::string_view class_name, const TypeInfo& base_type);

//! 名前を指定して指定基底クラス型でnewする (テンプレートヘルパー)
//! @param  [in]    class_name  クラス名
//! @tparam [in]    T           基底クラスの型
//! @return newされたクラスオブジェクト(失敗の場合はnullptr)
template <class T>
[[nodiscard]] T* CreateInstanceFromName(std::string_view class_name)
{
    return reinterpret_cast<T*>(CreateInstanceFromName(class_name, T::Type));
}

//@}
