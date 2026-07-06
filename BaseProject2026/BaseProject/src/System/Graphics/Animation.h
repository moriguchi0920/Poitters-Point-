//---------------------------------------------------------------------------
//! @file   Animation.h
//! @brief  アニメーション
//---------------------------------------------------------------------------
#pragma once

class AnimationModifier;
class ResourceAnimation;

//===========================================================================
//! アニメーションクラス
//===========================================================================
class Animation final
{
public:
	//! アニメーション定義オプション
	struct Desc
	{
		std::string name_;						//!< アニメーション名(任意)
		std::string file_path_;					//!< ファイルパス
		u32			animation_index_ = 0;		//!< ファイル内のアニメーション番号
		f32			animation_speed_ = 1.0f;	//!< アニメーションの再生速度(default:1.0f)
	};

	//----------------------------------------------------------
	//! @name   初期化
	//----------------------------------------------------------
	//@{

	//! デフォルトコンストラクタ
	Animation() = default;

	// コンストラクタ
	//! @param [in] desc          定義オプション配列の先頭アドレス
	//! @param [in] desc_count  定義オプション配列の個数
	Animation(const Animation::Desc* desc, size_t desc_count);

	// デストラクタ
	virtual ~Animation();

	// 作成
	//! @param [in] desc          定義オプション配列の先頭アドレス
	//! @param [in] desc_count  定義オプション配列の個数
	//! @retval true    成功(正常終了)
	//! @retval false   失敗(エラー終了)
	bool load(const Animation::Desc* desc, size_t desc_count);

	//! アニメーション操作クラスを作成
	//! @param [in] frame_index 対象になる関節フレーム番号
	std::shared_ptr<AnimationModifier> createAnimationModifier(s32 frame_index);

	//@}
	//----------------------------------------------------------
	//! @name   更新
	//----------------------------------------------------------
	//@{

	//  更新
	//! @param  [in]    dt  経過時間(単位:秒)
	void update(f32 dt);

	//@}
	//----------------------------------------------------------
	//! @name   設定
	//----------------------------------------------------------
	//@{

	//  アニメーション再生するモデルを設定する
	//! @param  [in]    model   3Dモデル(nullptrで解除)
	void bindModel(Model* model);

	//  アニメーションを再生する
	//! @param  [in]    name        アニメーション名
	//! @param  [in]    is_loop     ループ再生するかどうか(default:false)
	//! @param  [in]    blend_time  ブレンドする時間 (default:1.0秒)
	//! @param  [in]    start_time  再生開始時間(default:0.0f)
	//! @retval true    成功(正常終了)
	//! @retval false   失敗(nameで指定した名前が見つからなかった場合)
	bool play(std::string_view name, bool is_loop = false, f32 blend_time = 1.0f, f32 start_time = 0.0f);

	// アニメーションを一時停止する
	//! @param  [in]    active  停止フラグ(true:停止 false:再開)
	void pause(bool active = true);

	//@}
	//----------------------------------------------------------
	//! @name   取得
	//----------------------------------------------------------
	//@{

	//! [DxLib] モデル用MV1ハンドルを取得
	int model_mv1_handle() const;

	//! アニメーション番号を取得(アニメーション名指定)
	s32 animationIndex(std::string_view name) const;

	//! アニメーション定義を取得
	const Animation::Desc* desc(s32 animation_index) const;

	//! [DxLib] アニメーション用MV1ハンドルを取得
	int animation_mv1_handle(s32 animation_index) const;

	//  再生中かどうかを取得
	bool isPlaying() const;

	//  一時停止中かどうかを取得
	bool isPaused() const;

	// 初期化が正しく成功しているかどうかを取得
	bool isValid() const;

	// 利用可能な状態かどうか取得
	//! @note   利用可能になっていない状態でDxLibアニメーション関数を呼ぶと非同期ロードがブロッキングされます
	bool isActive() const;

	//@}
	//----------------------------------------------------------
	//! @name   copy/move禁止
	//----------------------------------------------------------
	//@{

private:
	Animation(const Animation&)		 = delete;
	Animation(Animation&&)			 = delete;
	void operator=(const Animation&) = delete;
	void operator=(Animation&&)		 = delete;

	//@}

private:
	std::wstring path_;							 //!< ファイルパス
	bool		 is_valid_		   = false;		 //!< 初期化が正しく成功しているかどうか
	Model*		 model_			   = nullptr;	 //!< 関連付けられているモデル
	int			 model_mv1_handle_ = -1;		 //!< [DxLib] 関連付けられているモデルのハンドル

	std::vector<Animation::Desc> descs_;					//!< アニメーション定義情報
	std::vector<int>			 animation_mv1_handles_;	//!< [DxLib] アニメーション定義情報と対になるアニメーションMV1ハンドル

	//! 名前逆引きテーブル (名前からDescのインデックス番号を取得)
	std::unordered_map<std::string, u32> name_table_;

	//! アニメーション操作クラス ([0]がデフォルトのメインクラス)
	std::vector<std::shared_ptr<AnimationModifier>> modifiers_;
};

//===========================================================================
//! アニメーション操作クラス
//===========================================================================
class AnimationModifier
{
public:
	//! コンストラクタ
	//! @param  [in]    owner   親のアニメーションクラス
	AnimationModifier(Animation* owner, s32 frame_index);

	//! デストラクタ
	~AnimationModifier();

	//  アニメーションを割り当て
	//! @param  [in]    context_index   コンテキスト番号
	bool attachAnimation(s32 context_index);

	//  アニメーション割り当てを解除
	//! @param  [in]    context_index   コンテキスト番号
	void detachAnimation(s32 context_index);

	//----------------------------------------------------------
	//! @name   更新
	//----------------------------------------------------------
	//@{

	//  更新
	//! @param  [in]    dt  経過時間(単位:秒)
	void update(f32 dt);

	//@}
	//----------------------------------------------------------
	//! @name   設定
	//----------------------------------------------------------
	//@{

	//  アニメーションを再生する
	//! @param  [in]    name        アニメーション名
	//! @param  [in]    is_loop     ループ再生するかどうか(default:false)
	//! @param  [in]    blend_time  ブレンドする時間 (default:1.0秒)
	//! @param  [in]    start_time  再生開始時間(default:0.0f)
	//! @retval true    成功(正常終了)
	//! @retval false   失敗(nameで指定した名前が見つからなかった場合)
	bool play(std::string_view name, bool is_loop = false, f32 blend_time = 1.0f, f32 start_time = 0.0f);

	// アニメーションを一時停止する
	//! @param  [in]    active  停止フラグ(true:停止 false:再開)
	void pause(bool active = true);

	//@}
	//----------------------------------------------------------
	//! @name   取得
	//----------------------------------------------------------
	//@{

	//  再生中かどうかを取得
	bool isPlaying() const;

	//  一時停止中かどうかを取得
	bool isPaused() const;

	//@}
	//----------------------------------------------------------
	//! @name   copy/move禁止
	//----------------------------------------------------------
	//@{

private:
	AnimationModifier(const AnimationModifier&) = delete;
	AnimationModifier(AnimationModifier&&)		= delete;
	void operator=(const AnimationModifier&)	= delete;
	void operator=(AnimationModifier&&)			= delete;

	//@}
private:
	Animation* owner_			   = nullptr;	 //!< 親のアニメーションクラス
	s32		   target_frame_index_ = -1;		 //!< 上書きする関節フレーム番号(-1の場合は上書きしない)

	//----------------------------------------------------------
	//! @name   再生中の情報
	//----------------------------------------------------------
	//@{

	bool is_paused_ = false;	//!< ポーズ中かどうか

	//! 再生中の情報構造体
	struct Context
	{
		bool is_playing_			 = false;	 //!< 再生中かどうか
		bool is_loop_				 = false;	 //!< ループ再生かどうか
		s32	 animation_index_		 = -1;		 //!< 現在再生中の番号(Animation::Descのインデックス番号)
		int	 animation_attach_index_ = -1;		 //!< [DxLib] アタッチされたスロット番号
		f32	 animation_total_time_	 = 0.0f;	 //!< 総再生時間
		f32	 play_time_				 = 0.0f;	 //!< 現在再生中の時間
		f32	 blend_ratio_			 = 0.0f;	 //!< ブレンド比(0.0f～1.0f)
	};

	static constexpr u32 CONTEXT_COUNT = 4;
	Context				 contexts_[CONTEXT_COUNT];	  //!< 構造体はアニメーションブレンドのため複数持つ
	f32					 blend_time_ = 1.0f;		  //!< ブレンドの補間時間

	//@}
};

//===========================================================================
//! アニメーション情報
//===========================================================================
class ResourceAnimation final
{
public:
	//! デフォルトコンストラクタ
	ResourceAnimation() = default;

	//  コンストラクタ
	//! @param  [in]    path    ファイルパス
	ResourceAnimation(std::string_view path);

	//  デストラクタ
	virtual ~ResourceAnimation();

	// 初期化が正しく成功しているかどうかを取得
	bool isValid() const;

	// 利用可能な状態かどうか取得
	//! @note   利用可能になっていない状態でDxLibアニメーション関数を呼ぶと非同期ロードがブロッキングされます
	bool isActive() const;

	// [DxLib] MV1ハンドルを取得
	operator int() const;

	//----------------------------------------------------------
	//! @name   copy/move禁止
	//----------------------------------------------------------
	//@{

private:
	ResourceAnimation(const ResourceAnimation&) = delete;
	ResourceAnimation(ResourceAnimation&&)		= delete;
	void operator=(const ResourceAnimation&)	= delete;
	void operator=(ResourceAnimation&&)			= delete;

	//@}

private:
	std::wstring	  path_;				  //!< モデルファイルへのパス
	int				  mv1_handle_ = -1;		  //!< [DxLib] MV1モデルハンドル (アニメーション用途)
	std::atomic<bool> active_	  = false;	  //!< アクティブ状態 true:利用可能 false:ロード未完了
};
