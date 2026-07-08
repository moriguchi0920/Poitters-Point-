//---------------------------------------------------------------------------
//! @file   Priority.h
//! @brief  優先設定
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! 優先度
//! 上位 priotiry 下位 sub_priority の値を一括して扱うソート可能な値です
//!
//! b63                              b0
//! +----------------+----------------+
//! |    priority    |  sub_priority  |
//! +----------------+----------------+
//! |<---          value_         --->|
//!
//===========================================================================
struct Priority
{
    //! コンストラクタ
    //! @param  [in]    priority    優先度
    //! @param  [in]    sub_priority    サブ優先度(任意利用)
    Priority(u32 priority = 0, u32 sub_priority = 0)
        : priority_(priority)
        , sub_priority_(sub_priority)
    {
    }

    //! コンストラクタ
    Priority(const Priority& other) { operator=(other); }

    //! 代入
    Priority& operator=(const Priority& other)
    {
        value_ = other.value_;
        return *this;
    }

    //! 比較
    bool operator<(const Priority& other) const { return value_ < other.value_; }

    //! 比較
    bool operator>(const Priority& other) const { return value_ > other.value_; }

    //! 比較
    bool operator==(const Priority& other) const { return value_ == other.value_; }

    //! 比較
    bool operator!=(const Priority& other) const { return value_ != other.value_; }

    //! キャスト
    operator u64() const { return value_; }

    union {
        struct
        {
            u32 sub_priority_;    //!< サブ優先度
            u32 priority_;        //!< 優先度
        };
        u64 value_;
    };
};
