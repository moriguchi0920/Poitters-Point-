//---------------------------------------------------------------------------
//! @file   ConstantBuffer.h
//! @brief  定数バッファ
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! 定数バッファ
//===========================================================================
class ConstantBuffer
{
public:
    // 定数バッファ更新のときに型指定する記法
    template <typename U>
    U* beginUpdate() const
    {
        void* p = beginUpdate();
        return reinterpret_cast<U*>(p);
    }

    //! バッファを更新開始
    //! @return 更新先のバッファのポインタ。このアドレスがバッファの先頭を指していますので書き換えてください。
    virtual void* beginUpdate() const = 0;

    //! バッファを更新終了
    virtual void endUpdate() const = 0;

    //! [DxLib] 定数バッファハンドルを取得
    //! @return 定数バッファハンドル
    virtual operator int() const = 0;

    virtual ~ConstantBuffer() = default;
};

//! 定数バッファを作成
//! @param  [in]    size    定数バッファのサイズ
std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t size);
