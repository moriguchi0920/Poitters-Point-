#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentTransform.h>
#include <System/Cereal.h>

#include <im-neo-sequencer/imgui_neo_sequencer.h>

//! @brief アニメーション内部構造体
struct AnimObject
{
    std::string name_;
    bool        loop_;

private:
    CEREAL_SAVELOAD(arc, ver) { arc(CEREAL_NVP(name_), CEREAL_NVP(loop_)); }
};

// sequencer用のオブジェクト
class SequenceObject : public IMatrix<SequenceObject>
{
public:
    virtual matrix& Matrix() { return object_matrix_; }

    //! @brief TransformのMatrix情報を取得します
    //! @return Transform の Matrix
    virtual const matrix& GetMatrix() const { return object_matrix_; }

    virtual std::shared_ptr<SequenceObject> SharedThis() { return nullptr; }

    virtual const matrix GetWorldMatrix() const { return object_matrix_; }

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const { return object_matrix_; }

public:
    std::string GetName()
    {
        // ImGuiにてname_の長さが変化するための対処
        return name_ + "##obj" + std::to_string(reinterpret_cast<uint64_t>(this));
    }

    std::string GetNameLabel() { return "NAME##name" + std::to_string(reinterpret_cast<uint64_t>(this)); }

    bool IsOpen() { return open_; }

    //! @brief フレーム情報からアニメーションを取得
    //! @param frame フレーム
    void SetAnimationFromFrame(int frame);

    //! @brief フレーム情報からエフェクトを取得
    //! @param frame フレーム
    void SetEffectFromFrame(int frame);

    //! @brief フレーム情報から姿勢(位置・回転・スケール)を取得
    //! @param frame フレーム
    matrix GetTransformFromFrame(int frame);

    //! @brief 三軸ギズモを表示する
    void ShowGuizmo();

    //! @brief データを並び替えます
    void Sort();

    //! @brief シーケンサ内部オブジェクトの更新
    //! @param playing 実行中か
    //! @param frame フレーム数
    void Update(bool playing, float frame);

    //! @brief GUI表示
    //! @param start スタートフレーム
    //! @param end 終了フレーム
    void GUI(uint32_t start, uint32_t end);

private:
    bool open_ = true;

    //! @brief ギズモの表示
    bool show_guizmo_ = false;

    //! @brief ギズモのオペレーション
    ImGuizmo::OPERATION guizmo_operation_ = ImGuizmo::OPERATION::TRANSLATE;

    //! @brief ギズモのモード
    ImGuizmo::MODE guizmo_mode_ = ImGuizmo::MODE::WORLD;

    //! @brief ギズモ番号
    int guizmo_index_ = -1;

    std::string           name_{};             //!< オブジェクトの名前
    std::vector<uint32_t> position_keys_{};    //!< ポジションキー
    std::vector<uint32_t> rotation_keys_{};    //!< ローテーションキー
    std::vector<uint32_t> scale_keys_{};       //!< スケールキー

    std::vector<float3> position_values_{};    //!< ポジション値
    std::vector<float3> rotation_values_{};    //!< ローテーション値
    std::vector<float3> scale_values_{};       //!< スケール値

    std::vector<uint32_t>   animation_keys_{};      //!< アニメーションキー
    std::vector<AnimObject> animation_values_{};    //!< アニメーション値

    std::string animation_name_{};               //!< アニメーション名
    int         animation_frame_old_ = -1;       //!< 前のフレーム
    int         animation_frame_     = -1;       //!< 現在のフレーム
    bool        animation_loop_      = false;    //!< ループするかの設定
    bool        animation_change_    = false;    //!< アニメーションが変わったか

    std::vector<uint32_t>   effect_keys_{};      //!< エフェクトキー
    std::vector<AnimObject> effect_values_{};    //!< エフェクト値

    int  effect_number_    = -1;       //!< エフェクト番号
    int  effect_frame_old_ = -1;       //!< 前のフレーム
    int  effect_frame_     = -1;       //!< 現在のフレーム
    bool effect_loop_      = false;    //!< ループするかの設定
    bool effect_change_    = false;    //!< エフェクトが変わったか

    matrix object_matrix_{
        float4{1, 0, 0, 0},
        float4{0, 1, 0, 0},
        float4{0, 0, 1, 0},
        float4{0, 0, 0, 1},
    };

    //! @brief ロードセーブ情報
    //! @param arc アーカイバ
    //! @param var バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        //! ロードセーブする変数
        arc(CEREAL_NVP(open_),    //
            CEREAL_NVP(name_),
            CEREAL_NVP(position_keys_),
            CEREAL_NVP(rotation_keys_),
            CEREAL_NVP(scale_keys_),
            CEREAL_NVP(position_values_),
            CEREAL_NVP(rotation_values_),
            CEREAL_NVP(scale_values_),
            CEREAL_NVP(animation_keys_),
            CEREAL_NVP(animation_values_),
            CEREAL_NVP(scale_values_),
            CEREAL_NVP(scale_values_),
            CEREAL_NVP(animation_name_),
            CEREAL_NVP(animation_frame_old_),
            CEREAL_NVP(animation_frame_),
            CEREAL_NVP(animation_change_),
            CEREAL_NVP(animation_loop_),
            CEREAL_NVP(effect_number_),
            CEREAL_NVP(effect_frame_old_),
            CEREAL_NVP(effect_frame_),
            CEREAL_NVP(effect_change_),
            CEREAL_NVP(effect_loop_),
            CEREAL_NVP(object_matrix_));
    }
};

CEREAL_REGISTER_TYPE(SequenceObject)

USING_PTR(ComponentSequencer);

//--------------------------------------------------------
//! @brief シーケンサコンポーネント
//--------------------------------------------------------
class ComponentSequencer : public Component
{
    friend class Object;

public:
    BP_COMPONENT_DECL(ComponentSequencer, u8"Sequencer機能クラス");

    ComponentSequencer() {}

    virtual void Init() override;          //!< 初期化
    virtual void Update() override;        //!< 更新
    virtual void PostUpdate() override;    //!< 更新後の処理
    virtual void Draw() override;          //!< デバッグ描画
    virtual void Exit() override;          //!< 終了処理
    virtual void GUI() override;           //!< GUI処理

    //! 再生処理
    void Play(int start_frame = 0, bool loop = false);

    //! 再生停止
    void Stop();

    //! 再生中かチェック
    bool IsPlaying() const;

    //---------------------------------------------------------------------------
    //! ステータス
    //---------------------------------------------------------------------------
    enum struct SequencerBit : u32
    {
        Initialized,    //!< 初期化済み
        Playing,        //!< プレイ中
        Loop,           //!< ループ指定
    };

    //! @brief シーケンサのステータス設定
    //! @param bit シーケンサBit
    //! @param on 有効無効の設定
    void SetSequencerStatus(SequencerBit bit, bool on) { sequencer_status_.set(bit, on); }

    //! @brief シーケンサのステータスの取得
    //! @param bit シーケンサBit
    //! @retval true  : 有効
    //! @retval false : 無効
    bool GetSequencerStatus(SequencerBit bit) const { return sequencer_status_.is(bit); }

#if 0
	uint32_t GetStartFrame()
	{
		return start_frame_;
	}
	void SetStartFrame( uint32_t start )
	{
		start_frame_ = start;
	}
	uint32_t GetEndFrame()
	{
		return start_frame_;
	}
	void SetEndFrame( uint32_t end )
	{
		end_frame_ = end;
	}
	float GetCurrentFrame()
	{
		return current_frame_;
	}
	void SetCurrentFrame( float current )
	{
		current_frame_ = current;
	}
#endif

private:
    Status<SequencerBit> sequencer_status_;    //!< 状態

    std::vector<std::shared_ptr<SequenceObject>> objects_;

    uint32_t start_frame_   = 0;
    uint32_t end_frame_     = 60 * 5;
    float    current_frame_ = 0;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));    //< オーナー
        arc(cereal::make_nvp("sequencer_status", sequencer_status_.get()));
        arc(CEREAL_NVP(start_frame_), CEREAL_NVP(end_frame_), CEREAL_NVP(current_frame_));
        arc(CEREAL_NVP(objects_));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentSequencer)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentSequencer)
