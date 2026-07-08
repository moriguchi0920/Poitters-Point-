//---------------------------------------------------------------------------
//! @file   JobScheduler.h
//! @brief  ジョブスケジューラー
//---------------------------------------------------------------------------
#pragma once

//===========================================================================
//! ジョブハンドル
//! @note 登録されたジョブを管理するためのハンドルです
//===========================================================================
struct JobHandle
{
protected:
    u32 value_ = 0;    //!< ハンドル値
};

//===========================================================================
//! ジョブグループ
//===========================================================================
class JobGroup final : noncopyable, nonmovable
{
public:
    //  ジョブを一括実行します
    void execute();

    //  ジョブを登録します
    //! @param  [in]    priority    実行優先順位
    //! @param  [in]    func        実行する関数
    //! @return 登録されたジョブハンドル (0なら登録失敗)
    //! @note 一度登録状態になったジョブは常に登録されたままになります。
    //
    //  【使用例】
    // registerJob(Priority(カテゴリーTYPE, TYPEの中の優先度), [](){ ラムダ式 })
    JobHandle registerJob(const Priority& priority, const std::function<void()>& func);

    //  ジョブを登録解除します
    //! @param  [in]    ジョブハンドル
    void unregisterJob(const JobHandle& job_handle);

    //  対象のジョブを有効にする
    //! @param  [in]    job_handle  ジョブハンドル
    //! @note 初期状態は「有効」になっていますが、disableJob()で無効化されたジョブを再び有効化できます。
    void enableJob(const JobHandle& job_handle);

    //  対象のジョブを無効にする
    //! @param  [in]    job_handle  ジョブハンドル
    //! @note 無効化されたジョブは登録状態のまま呼び出しが停止されます
    void disableJob(const JobHandle& job_handle);

public:
    // コンストラクタ
    //! @param  [in]    job_max_count   ジョブ登録最大数
    JobGroup(u32 job_max_count = 10000);

    //! デストラクタ
    virtual ~JobGroup() = default;

private:
    using Job = std::pair<Priority, std::function<void()>>;

    std::vector<Job> jobs_;             //!< ジョブ実体
    std::vector<u32> job_free_list_;    //!< 空いているジョブスロットの一覧
    std::vector<u32> job_used_list_;    //!< 使用中ジョブスロットの一覧
};
