//---------------------------------------------------------------------------
//! @file   ConstantBuffer.cpp
//! @brief  定数バッファ
//---------------------------------------------------------------------------
#include "ConstantBuffer.h"

//===========================================================================
//! 定数バッファ
//===========================================================================
class ConstantBufferImpl final : public ConstantBuffer, noncopyable, nonmovable
{
public:
    //! デフォルトコンストラクタ
    ConstantBufferImpl() = default;

    //! コンストラクタ(サイズ指定)
    //! @param [in] size    定数バッファサイズ (4の倍数が必要。16の倍数を推奨)
    ConstantBufferImpl(size_t size)
    {
        if(size > 0) {
            constant_buffer_ = CreateShaderConstantBuffer(static_cast<int>(size));
        }
    }

    //! デストラクタ
    virtual ~ConstantBufferImpl()
    {
        // 定数バッファを解放
        if(constant_buffer_ != -1) {
            DeleteShaderConstantBuffer(constant_buffer_);
        }
    }

    //! バッファを更新開始
    //! @return 更新先のバッファのポインタ。このアドレスがバッファの先頭を指していますので書き換えてください。
    virtual void* beginUpdate() const override
    {
        // 更新に必要なワークメモリの場所を取得
        return GetBufferShaderConstantBuffer(constant_buffer_);
    }

    //! バッファを更新終了
    virtual void endUpdate() const override
    {
        // 定数バッファワークメモリをGPU側へ転送
        UpdateShaderConstantBuffer(constant_buffer_);
    }

    //! [DxLib] 定数バッファハンドルを取得
    //! @return 定数バッファハンドル
    virtual operator int() const override { return constant_buffer_; }

private:
    int constant_buffer_ = -1;    //!< [DxLib] 定数バッファハンドルを取得
};

//---------------------------------------------------------------------------
//! 定数バッファを作成
//---------------------------------------------------------------------------
std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t size)
{
    auto p = std::make_shared<ConstantBufferImpl>(size);
    if(*p == 0) {
        p.reset();
    }
    return p;
}
