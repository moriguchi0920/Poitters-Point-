namespace Sample::GameSample {

namespace HP {

class Gauge final
{
    const static int DEF_HP_VALUE = 100;

public:
    //! @brief ゲージコンストラクタ
    //! @param hp トータルゲージ量
    Gauge(float hp = DEF_HP_VALUE);

    ~Gauge();

    //! @brief 更新
    void Update();

    //! @brief ゲージ表示
    void Draw();

    //! @brief ゲージ全回復
    void Reset();

    //! @name オペレーター
    //! @{

    Gauge& operator++();
    Gauge  operator++(int);
    Gauge& operator--();
    Gauge  operator--(int);

    void operator-=(int value);
    void operator+=(int value);

    bool operator<=(int value);
    bool operator<(int value);
    bool operator>=(int value);
    bool operator>(int value);
    bool operator==(int value);
    bool operator!=(int value);

    float& Value() { return hp_; }

    //! }@

private:
    float3 pos_           = float3(0, 0, 0);    //!< 位置
    float  hp_            = DEF_HP_VALUE;       //!< HP（黄色のバー）
    float  org_hp_        = DEF_HP_VALUE;       //!< HPの基本量
    float  damage_        = 0.0f;               //!< ダメージ（赤色のバー）
    float  damage_frame_  = 0;                  //!< ダメージが与えられた後、赤いゲージが減るまで待つ用
    float  damage_delta_  = 0;                  //!< ダメージが与えられた後、赤いゲージが減る量用
    float  recover_       = 0.0f;               //!< 回復（緑のバー）
    float  recover_frame_ = 0;                  //!< 回復が与えられた後、緑ゲージが減るまで待つ用
    float  recover_delta_ = 0;                  //!< 回復が与えられた後、緑ゲージが減る量用

    //! @brief 割合に変更する
    //! @param value 現在のゲージ量
    //! @return 割合
    float ToPerValue(float value);

    //! @brief ダメージゲージ処理
    void procDamageGauge(float delta);
    //! @brief リカバーゲージ処理
    void procRecoverGauge(float delta);
};
}    // namespace HP

}    // namespace Sample::GameSample
