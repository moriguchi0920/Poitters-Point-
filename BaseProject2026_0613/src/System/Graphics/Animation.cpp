//---------------------------------------------------------------------------
//! @file   Animation.cpp
//! @brief  アニメーション
//---------------------------------------------------------------------------
#include "Animation.h"

namespace {

//! アニメーションリソースプール
std::unordered_map<std::string, std::shared_ptr<ResourceAnimation>> resource_pool;

}    // namespace

//---------------------------------------------------------------------------
//! コントラクタ
//---------------------------------------------------------------------------
Animation::Animation(const Animation::Desc* desc, size_t desc_count)
{
    load(desc, desc_count);
}

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
Animation::~Animation()
{
    // モデルが関連付けられている場合は相手の設定を解除
    if(model_) {
        model_->bindAnimation(nullptr);
    }

    // アニメーションMV1を解放
    for(auto& x : animation_mv1_handles_) {
        if(x == -1)
            continue;

        DxLib::MV1DeleteModel(x);
        x = -1;
    }
}

//---------------------------------------------------------------------------
//! 作成
//---------------------------------------------------------------------------
bool Animation::load(const Animation::Desc* desc, size_t desc_count)
{
    is_valid_ = true;    // 先に一旦trueにする

    // パラメーター初期化
    for(u32 i = 0; i < desc_count; ++i) {
        const auto& x = desc[i];

        //------------------------------------------------------
        // アニメーションリソース作成
        // 既に同名ファイルがある場合は共有
        //------------------------------------------------------
        const std::string& resource_path = x.file_path_;

        auto it = resource_pool.find(resource_path);
        if(it == resource_pool.end()) {    // 新規登録
            resource_pool[resource_path] = std::make_shared<ResourceAnimation>(resource_path);
        }

        // 共有
        std::shared_ptr<ResourceAnimation>& resource = resource_pool[resource_path];

        descs_.push_back(x);
        animation_mv1_handles_.push_back(MV1DuplicateModel(*resource));

        if(resource->isValid() == false) {
            is_valid_ = false;    // 一度でもエラーの場合はfalse
        }

        // 名前逆引きテーブルに登録
        name_table_[desc[i].name_] = i;
    }

    // デフォルトのAnimationModifierを作成
    s32 frame_index = -1;
    createAnimationModifier(frame_index);

    return is_valid_;
}

//---------------------------------------------------------------------------
//! アニメーション操作クラスを作成
//---------------------------------------------------------------------------
std::shared_ptr<AnimationModifier> Animation::createAnimationModifier(s32 frame_index)
{
    auto p = std::make_shared<AnimationModifier>(this, frame_index);
    modifiers_.push_back(p);
    return p;
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void Animation::update(f32 dt)
{
    assert(model_ && "アニメーションにモデルが設定されていません");

    for(auto& modifier : modifiers_) {
        modifier->update(dt);
    }
}

//---------------------------------------------------------------------------
//! アニメーション再生するモデルを設定する
//---------------------------------------------------------------------------
void Animation::bindModel(Model* model)
{
    if(model == model_)
        return;

    // [DxLib]モデルとアニメーションの関連付けを解除
    modifiers_.clear();

    // 旧モデルを解除
    auto* last = model_;
    if(last) {
        model_            = nullptr;
        model_mv1_handle_ = -1;

        last->bindAnimation(nullptr);
    }

    // 新モデルに登録
    model_ = model;
    if(model_) {
        model_mv1_handle_ = *model_;
        model_->bindAnimation(this);

        // デフォルトのAnimationModifierを作成
        s32 frame_index = -1;
        createAnimationModifier(frame_index);
    }
}

//---------------------------------------------------------------------------
//! アニメーションを再生する
//---------------------------------------------------------------------------
bool Animation::play(std::string_view name, bool is_loop, f32 blend_time, f32 start_time)
{
    return modifiers_[0]->play(name, is_loop, blend_time, start_time);
}

//---------------------------------------------------------------------------
//! アニメーションを停止する
//---------------------------------------------------------------------------
void Animation::pause(bool active)
{
    modifiers_[0]->pause(active);
}

//---------------------------------------------------------------------------
//! [DxLib] モデル用MV1ハンドルを取得
//---------------------------------------------------------------------------
int Animation::model_mv1_handle() const
{
    return model_mv1_handle_;
}

//---------------------------------------------------------------------------
//! アニメーション番号を取得(アニメーション名指定)
//---------------------------------------------------------------------------
s32 Animation::animationIndex(std::string_view name) const
{
    std::string name_string(name);
    auto        it = name_table_.find(name_string);

    if(it == name_table_.end()) {
        return -1;    // 見つからなかった場合
    }

    // アニメーション番号
    auto animation_index = it->second;
    return animation_index;
}

//---------------------------------------------------------------------------
//! アニメーション定義を取得(アニメーション番号指定)
//---------------------------------------------------------------------------
const Animation::Desc* Animation::desc(s32 index) const
{
    return &descs_[index];
}

//---------------------------------------------------------------------------
//! [DxLib] アニメーション用MV1ハンドルを取得
//---------------------------------------------------------------------------
int Animation::animation_mv1_handle(s32 index) const
{
    return animation_mv1_handles_[index];
}

//---------------------------------------------------------------------------
//!  再生中かどうかを取得
//---------------------------------------------------------------------------
bool Animation::isPlaying() const
{
    return modifiers_[0]->isPlaying();
}

//---------------------------------------------------------------------------
//!  一時停止中かどうかを取得
//---------------------------------------------------------------------------
bool Animation::isPaused() const
{
    return modifiers_[0]->isPaused();
}

//---------------------------------------------------------------------------
//! 初期化が正しく成功しているかどうかを取得
//---------------------------------------------------------------------------
bool Animation::isValid() const
{
    return is_valid_;
}

//---------------------------------------------------------------------------
//! 利用可能な状態かどうか取得
//---------------------------------------------------------------------------
bool Animation::isActive() const
{
    return isValid();
}

//===========================================================================
//  AnimationModifier
//===========================================================================

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
AnimationModifier::AnimationModifier(Animation* owner, s32 frame_index)
{
    owner_              = owner;
    target_frame_index_ = frame_index;
}

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
AnimationModifier::~AnimationModifier()
{
    for(u32 i = 0; i < CONTEXT_COUNT; ++i) {
        detachAnimation(i);
    }
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void AnimationModifier::update(f32 dt)
{
    // 再生していない場合
    if(isPlaying() == false)
        return;

    // ポーズ中の場合
    if(is_paused_)
        return;

    //----------------------------------------------------------
    // アニメーションを進める
    //----------------------------------------------------------
    float blend_ratio_total = 0.0f;
    for(u32 i = 0; i < std::size(contexts_); ++i) {
        auto& c = contexts_[i];

        if(c.animation_index_ == -1)
            continue;

        auto* desc             = owner_->desc(c.animation_index_);
        auto  model_mv1_handle = owner_->model_mv1_handle();

        // アニメーション再生時間を進める
        c.play_time_ += dt * 30.0f * desc->animation_speed_;

        if(c.is_loop_) {
            // アニメーション再生時間がアニメーションの総時間を越えていたらループさせる
            if(c.animation_total_time_ <= c.play_time_) {
                c.play_time_ -= c.animation_total_time_;
            }
        }
        else {
            // アニメーション再生時間がアニメーションの総時間を越えていたら停止
            if(c.animation_total_time_ <= c.play_time_) {
                c.play_time_  = c.animation_total_time_;
                c.is_playing_ = false;
            }
        }

        // 新しいアニメーション再生時間をセット
        DxLib::MV1SetAttachAnimTime(model_mv1_handle, c.animation_attach_index_, c.play_time_);

        //----------------------------------------------------------
        // アニメーションブレンド
        //----------------------------------------------------------

        // ブレンドを進める
        c.blend_ratio_ += ((1.0f / blend_time_) * dt) * ((i == 0) ? +1.0f : -1.0f);
        c.blend_ratio_  = std::clamp(c.blend_ratio_, 0.0f, 1.0f);

        blend_ratio_total += c.blend_ratio_;
    }

    //----------------------------------------------------------
    // アニメーションブレンド処理
    //----------------------------------------------------------
    blend_ratio_total = std::max(FLT_EPSILON, blend_ratio_total);

    auto model_mv1_handle = owner_->model_mv1_handle();

    // メインのアニメーション再生はブレンドレートを通常通り設定する
    if(target_frame_index_ == -1) {
        for(u32 i = 0; i < std::size(contexts_); ++i) {
            auto& c     = contexts_[i];
            float ratio = c.blend_ratio_;

            if(c.animation_attach_index_ == -1)
                continue;

            DxLib::MV1SetAttachAnimBlendRate(model_mv1_handle, c.animation_attach_index_, ratio / blend_ratio_total);
        }
    }
    else {
        // 上書き用のアニメーション再生はブレンドレートはすべて0.0fにしてから部分設定する
        for(u32 i = 0; i < std::size(contexts_); ++i) {
            auto& c     = contexts_[i];
            float ratio = c.blend_ratio_;

            if(c.animation_attach_index_ == -1)
                continue;

            DxLib::MV1SetAttachAnimBlendRate(model_mv1_handle, c.animation_attach_index_, 0.0f);
        }

        // 【部分設定】他のアニメーションIndexの上書きブレンドを0.0fにする
        // TODO:ここでは固定で16個を設定していますが有効なアニメーションIndexを全列挙するほうが好ましい。
        for(u32 j = 0; j < 16; ++j) {
            DxLib::MV1SetAttachAnimBlendRateToFrame(model_mv1_handle, j, target_frame_index_, 0.0f, true);
        }

        // 【部分設定】 上書きブレンドを設定
        for(u32 i = 0; i < std::size(contexts_); ++i) {
            auto& c     = contexts_[i];
            float ratio = c.blend_ratio_;

            if(c.animation_attach_index_ == -1)
                continue;

            DxLib::MV1SetAttachAnimBlendRateToFrame(model_mv1_handle, c.animation_attach_index_, target_frame_index_, ratio / blend_ratio_total, true);
        }
    }

    for(u32 i = 0; i < std::size(contexts_); ++i) {
        auto& c     = contexts_[i];
        float ratio = c.blend_ratio_;

        if(c.animation_attach_index_ == -1)
            continue;

        // [0]はプライマリアニメーションのためスキップ
        if(i == 0)
            continue;

        // 補間完了後は補間元のアニメーションを停止
        if(std::fabsf(c.blend_ratio_) < FLT_EPSILON) {    // == 0.0f
            detachAnimation(i);
            c.is_playing_ = false;
        }
    }
}

//---------------------------------------------------------------------------
//! アニメーションを再生する
//---------------------------------------------------------------------------
bool AnimationModifier::play(std::string_view name, bool is_loop, f32 blend_time, f32 start_time)
{
    // ブレンドの速度
    blend_time_ = std::max(FLT_EPSILON, blend_time);    // 0.0fにならないように

    //----------------------------------------------------------
    // [DxLib] 設定されているアニメーションを一旦解除
    //----------------------------------------------------------
    for(u32 i = 0; i < CONTEXT_COUNT; ++i) {
        detachAnimation(i);
    }

    // 補間のために現在のアニメーションを補間前として設定
    // [0] = 現在のアニメーション
    // [1以降] = 以前のアニメーション
    for(s32 i = CONTEXT_COUNT - 2; i >= 0; --i) {
        contexts_[i + 1] = contexts_[i];
    }

    //----------------------------------------------------------
    // 新規再生
    //----------------------------------------------------------
    {
        // 名前からアニメーション番号を取得
        s32 animation_index = owner_->animationIndex(name);
        if(animation_index == -1) {
            return false;
        }

        auto& c = contexts_[0];

        // アニメーション番号
        c.animation_index_ = animation_index;
        // 現在の時間
        c.play_time_ = start_time;    // 開始位置は start_time から

        c.is_playing_  = true;
        c.is_loop_     = is_loop;
        c.blend_ratio_ = 0.0f;
    }

    //----------------------------------------------------------
    // アニメーション再生を設定
    //----------------------------------------------------------
    for(u32 i = 0; i < CONTEXT_COUNT; ++i) {
        attachAnimation(i);
    }

    // ポーズ解除
    is_paused_ = false;

    // 強制的に再更新(時間は進めない)
    // これを実行することで1フレームだけ不定値になってT-poseになる現象を回避
    update(0.0f);

    return true;
}

//---------------------------------------------------------------------------
//! アニメーションを停止する
//---------------------------------------------------------------------------
void AnimationModifier::pause(bool active)
{
    is_paused_ = active;
}

//---------------------------------------------------------------------------
//!  再生中かどうかを取得
//---------------------------------------------------------------------------
bool AnimationModifier::isPlaying() const
{
    return contexts_[0].is_playing_;
}

//---------------------------------------------------------------------------
//!  一時停止中かどうかを取得
//---------------------------------------------------------------------------
bool AnimationModifier::isPaused() const
{
    return is_paused_;
}

//---------------------------------------------------------------------------
//! アニメーションを割り当て
//---------------------------------------------------------------------------
bool AnimationModifier::attachAnimation(s32 context_index)
{
    auto& c = contexts_[context_index];

    if(c.animation_index_ == -1)
        return false;

    auto* desc                 = owner_->desc(c.animation_index_);
    auto  animation_mv1_handle = owner_->animation_mv1_handle(c.animation_index_);    // [DxLib] アニメーションのMV1ハンドル
    auto  model_mv1_handle     = owner_->model_mv1_handle();                          // [DxLib] モデルMV1ハンドル

    // モデルにアニメーションを設定
    auto animation_index = desc->animation_index_;    // MV1ファイル内のアニメーション番号

    c.animation_attach_index_ = DxLib::MV1AttachAnim(model_mv1_handle, animation_index, animation_mv1_handle, true);

    // アニメーション総再生時間を取得
    c.animation_total_time_ = DxLib::MV1GetAttachAnimTotalTime(model_mv1_handle, c.animation_attach_index_);

    // アニメーション再生時間を初期化
    DxLib::MV1SetAttachAnimTime(model_mv1_handle, c.animation_attach_index_, c.play_time_);

    return true;
}

//---------------------------------------------------------------------------
//! アニメーション割り当てを解除
//---------------------------------------------------------------------------
void AnimationModifier::detachAnimation(s32 context_index)
{
    auto model_mv1_handle = owner_->model_mv1_handle();    // [DxLib] モデルMV1ハンドル

    if(contexts_[context_index].animation_attach_index_ != -1) {
        DxLib::MV1DetachAnim(model_mv1_handle, contexts_[context_index].animation_attach_index_);
        contexts_[context_index].animation_attach_index_ = -1;
    }
}

//===========================================================================
//  ResourceAnimation
//===========================================================================

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
ResourceAnimation::ResourceAnimation(std::string_view path)
{
    //-----------------------------------------------------------------------
    // MV1モデルアニメーションの読み込み
    //-----------------------------------------------------------------------
    std::string resource_path(path);

    // パスの保存
    path_ = convertTo(resource_path);

    // アニメーションが含まれるモデルデータを読み込み
    mv1_handle_ = DxLib::MV1LoadModel(resource_path.c_str());

    // ハンドルの非同期読み込み処理が完了したら呼ばれる関数
    auto finish_callback = []([[maybe_unused]] int mv1_handle, void* data) {
        auto* resource = reinterpret_cast<ResourceAnimation*>(data);

        // アクティブフラグを設定
        resource->active_ = true;
    };

    //----------------------------------------------------------
    // 非同期読み込み
    //----------------------------------------------------------
    SetUseASyncLoadFlag(true);
    {
        // モデルの読み込み
        mv1_handle_ = DxLib::MV1LoadModel(resource_path.c_str());
        DxLib::SetASyncLoadFinishCallback(mv1_handle_, (void (*)(int, void*))finish_callback, this);
    }
    SetUseASyncLoadFlag(false);
}

//---------------------------------------------------------------------------
//! デストラクタ
//---------------------------------------------------------------------------
ResourceAnimation::~ResourceAnimation()
{
    // アニメーションを解放
    if(mv1_handle_ != -1) {
        DxLib::MV1DeleteModel(mv1_handle_);
    }
}

//---------------------------------------------------------------------------
//! 初期化が正しく成功しているかどうかを取得
//---------------------------------------------------------------------------
bool ResourceAnimation::isValid() const
{
    return mv1_handle_ != -1;
}

//---------------------------------------------------------------------------
//! 利用可能な状態かどうか取得
//---------------------------------------------------------------------------
bool ResourceAnimation::isActive() const
{
    return active_;
}

//---------------------------------------------------------------------------
//! [DxLib] MV1ハンドルを取得
//---------------------------------------------------------------------------
ResourceAnimation::operator int() const
{
    return mv1_handle_;
}
