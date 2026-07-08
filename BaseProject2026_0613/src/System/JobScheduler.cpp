//---------------------------------------------------------------------------
//! @file   JobScheduler.cpp
//! @brief  ジョブスケジューラー
//---------------------------------------------------------------------------
#include "JobScheduler.h"

struct JobHandleImpl : public JobHandle
{
public:
    //! コンストラクタ
    JobHandleImpl(u32 value) { value_ = value; }

    //! ハンドル値を取得
    u32 value() const { return value_; }
};

//===========================================================================
// ジョブグループ JobGroup
//===========================================================================

//---------------------------------------------------------------------------
//! ジョブを一括実行します
//---------------------------------------------------------------------------
void JobGroup::execute()
{
    std::vector<Job> sorted_job;

    // 使用中リストを探索して直列に配置しなおす
    // 途中に未使用領域があっても一列に詰められる
    for(u32 index : job_used_list_) {
        auto& job = jobs_[index];

        // 一時無効になっているジョブは実行しない
        if(job.first.priority_ & 0x80000000ul) {
            continue;
        }

        sorted_job.push_back(job);
    }

    //----------------------------------------------------------
    // ジョブを実行優先順位でソート
    //----------------------------------------------------------
    auto compare = [&](const Job& a, const Job& b) { return (a.first < b.first); };

    std::stable_sort(std::begin(sorted_job), std::end(sorted_job), compare);

    //----------------------------------------------------------
    // 実行
    //----------------------------------------------------------
    for(auto& job : sorted_job) {
        job.second();
    }
}

//---------------------------------------------------------------------------
//! ジョブを登録します
//---------------------------------------------------------------------------
JobHandle JobGroup::registerJob(const Priority& priority, const std::function<void()>& func)
{
    if(job_free_list_.empty()) {
        assert(false);
        return JobHandleImpl(0);    // エラー
    }
    // アクセスする配列番号を取得 (空き番号を取得)
    u32 index = job_free_list_.back();
    job_free_list_.pop_back();

    // 登録
    jobs_[index] = std::move(Job{priority, func});
    job_used_list_.push_back(index);    // 使用中にする

    return JobHandleImpl(index | (0xbeef << 16));    // 上位16ビットをIDにすることで非0にする
}

//---------------------------------------------------------------------------
//! ジョブを登録解除します
//---------------------------------------------------------------------------
void JobGroup::unregisterJob(const JobHandle& job_handle)
{
    auto value = static_cast<const JobHandleImpl&>(job_handle).value();

    assert((value >> 16) == 0xbeef);    // ハンドルかどうかチェック
    u32 index = value & 0xfffful;

    // 番号がある場所を検索
    auto it = std::find(job_used_list_.begin(), job_used_list_.end(), index);
    if(it == job_used_list_.end()) {
        // 使用中リスト内に見つからなかった場合（二重解放の疑いがある）
        assert(false);
        return;
    }
    job_used_list_.erase(it);

    // 返却
    job_free_list_.push_back(index);
}

//---------------------------------------------------------------------------
//! 対象のジョブを有効にする
//---------------------------------------------------------------------------
void JobGroup::enableJob(const JobHandle& job_handle)
{
    auto value = static_cast<const JobHandleImpl&>(job_handle).value();

    assert((value >> 16) == 0xbeef);    // ハンドルかどうかチェック
    u32 index = value & 0xfffful;

    jobs_[index].first.priority_ &= ~0x80000000ul;
}

//---------------------------------------------------------------------------
//! 対象のジョブを無効にする
//---------------------------------------------------------------------------
void JobGroup::disableJob(const JobHandle& job_handle)
{
    auto value = static_cast<const JobHandleImpl&>(job_handle).value();

    assert((value >> 16) == 0xbeef);    // ハンドルかどうかチェック
    u32 index = value & 0xfffful;

    jobs_[index].first.priority_ |= 0x80000000ul;
}

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
JobGroup::JobGroup(u32 job_max_count)
    : jobs_(job_max_count)
{
    // 未使用リストに通し番号(すべての番号)を設定しておく
    for(u32 i = 0; i < job_max_count; ++i) {
        job_free_list_.push_back(i);
    }
}
