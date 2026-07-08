//---------------------------------------------------------------------------
//! @file   SceneInputPad.cpp
//! @brief  パッド入力サンプルシーン
//---------------------------------------------------------------------------
#include "SceneInputPad.h"

// 「【パッド入力】」と記載しているところがサンプルソースです
namespace {
struct Pos2
{
    f32 x_ = 0;
    f32 y_ = 0;
};

// プレイヤークラス
class Player
{
public:
    Player(PAD_NO pad_no)
        : pad_no_(pad_no)
    {
        u32       u32_pad_no  = convertPadNo(pad_no_);
        const u32 MAX_PAD_NUM = static_cast<u32>(PAD_NO::PAD_NO_MAX);

        Pos2 pos_list[MAX_PAD_NUM] = {
            { left(),   up()},
            {right(),   up()},
            { left(), down()},
            {right(), down()}
        };
        u32 color_list[MAX_PAD_NUM] = {RED, GREEN, WHITE, SKYBLUE};

        pos_   = pos_list[u32_pad_no];
        color_ = color_list[u32_pad_no];

        dash_pos_ = pos_;
    }
    ~Player()
    {
        // 何もすることはない
    }

    void update()
    {
        updateCoolTime();

        if(is_dash_ == true) {
            updateDash();
            return;
        }

        move();

        inputDirectionDash();
        if(canDash() == true) {
            setDash();
        }

        rePos();
    }
    void render()
    {
        // 本体
        DrawCircleAA(pos_.x_, pos_.y_, r(), 64, color_);

        // ダッシュ可能のチャージ用
        for(int i = 360 / 36 - cool_time_; i > 0; --i) {
            Pos2 dot_pos = {
                pos_.x_ + (r() + 10) * cosf(D2R((float)((i - 1) * 36) - 90)),
                pos_.y_ + (r() + 10) * sinf(D2R((float)((i - 1) * 36) - 90)),
            };
            DrawCircleAA(dot_pos.x_, dot_pos.y_, 1, 32, color_);
        }

        // ダッシュ方向用
        if(isInputDirectionKey() == true) {
            DrawCircleAA(pos_.x_ + (r() - 8) * cosf(D2R(dash_angle_)), pos_.y_ + (r() - 8) * sinf(D2R(dash_angle_)), 8, 4, GetColor(125, 125, 125));
        }

        // 王冠所持を分からせる用
        if(is_king_ == true) {
            DrawCircleAA(pos_.x_, pos_.y_, 15, 64, GetColor(255, 215, 0));
        }
    }

    // 座標
    const Pos2& pos() const { return pos_; }
    // 半径
    f32 r() const { return 30.0f; }

    bool isKing() const { return is_king_; }
    void setKing() { is_king_ = true; }
    void unsetKing() { is_king_ = false; }

private:
    Pos2 pos_;         // 座標
    Pos2 dash_pos_;    // ダッシュ時の目標座標

    PAD_NO pad_no_ = PAD_NO::PAD_NO1;    // パッドの番号

    u32 color_ = 0;    // 色

    f32 dash_angle_  = 0;    // ダッシュする方向
    u32 cool_time_   = 0;    // クールタイム
    u32 charge_time_ = 0;    // チャージの時間計測用

    bool is_dash_ = false;    // ダッシュ中かどうか
    bool is_king_ = false;    // 王冠を所持しているかどうか

    f32 right() { return (f32)WINDOW_W - r(); }
    f32 left() { return r(); }
    f32 down() { return (f32)WINDOW_H - r(); }
    f32 up() { return r(); }
    // enumのPAD番号をu32に変換
    u32 convertPadNo(PAD_NO pad_no) { return static_cast<u32>(pad_no); }

    // 通常移動
    //  【パッド入力】左上スティックでの通常移動
    void move()
    {
        constexpr float max_speed = 0.5f;
#if 1
        float2 vec = {max_speed * GetPadInputAnalogLX(pad_no_), max_speed * GetPadInputAnalogLY(pad_no_)};
        float  len = min(length(vec), max_speed);
        if(len > 0.0f)
            vec = normalize(vec) * len;

        pos_.x_ += vec.x;
        pos_.y_ += vec.y;
#else
        pos_.x_ += max_speed * GetPadInputAnalogLX(pad_no_);
        pos_.y_ += max_speed * GetPadInputAnalogLY(pad_no_);
#endif
    }
    // 座標の調整用
    void rePos()
    {
        pos_.x_ = std::max(pos_.x_, r());
        pos_.y_ = std::max(pos_.y_, r());
        pos_.x_ = std::min(pos_.x_, WINDOW_W - r());
        pos_.y_ = std::min(pos_.y_, WINDOW_H - r());
    }

    // ++++++++++++++++++++++++++++++++++++
    //  ダッシュ関係の処理
    // ++++++++++++++++++++++++++++++++++++
    // 方向キーのいずれかが入力されているかどうか
    bool isInputDirectionKey() const
    {
        if(IsPadRepeat(PAD_ID::PAD_D_UP, pad_no_) || IsPadRepeat(PAD_ID::PAD_D_DOWN, pad_no_) || IsPadRepeat(PAD_ID::PAD_D_LEFT, pad_no_) ||
           IsPadRepeat(PAD_ID::PAD_D_RIGHT, pad_no_)) {
            return true;
        }
        return false;
    }
    // ダッシュの角度入力用
    //  【パッド入力】パッド左下の方向キーでダッシュする4方向を決める
    void inputDirectionDash()
    {
        if(IsPadRepeat(PAD_ID::PAD_D_UP, pad_no_)) {
            dash_angle_ = 270;
        }
        else if(IsPadRepeat(PAD_ID::PAD_D_RIGHT, pad_no_)) {
            dash_angle_ = 0;
        }
        else if(IsPadRepeat(PAD_ID::PAD_D_DOWN, pad_no_)) {
            dash_angle_ = 90;
        }
        else if(IsPadRepeat(PAD_ID::PAD_D_LEFT, pad_no_)) {
            dash_angle_ = 180;
        }
    }

    // ダッシュのクールタイムのアップデート用
    void updateCoolTime()
    {
        if(cool_time_ < 1)
            return;

        charge_time_++;
        charge_time_ %= 30;
        if(charge_time_ == 0) {
            --cool_time_;
        }
    }
    // ダッシュ中の処理
    void updateDash()
    {
        pos_.x_ += (dash_pos_.x_ - pos_.x_) * 0.15f;
        pos_.y_ += (dash_pos_.y_ - pos_.y_) * 0.15f;

        f32 x_dist = pos_.x_ - dash_pos_.x_;
        f32 y_dist = pos_.y_ - dash_pos_.y_;
        f32 dist   = sqrtf(x_dist * x_dist + y_dist * y_dist);
        if(dist < 5.0f) {
            pos_     = dash_pos_;
            is_dash_ = false;
        }

        bool is_over_screen = (pos_.x_ < 0 + r()) || (pos_.y_ < 0 + r() - 1) || (pos_.x_ > WINDOW_W - r()) || (pos_.y_ > WINDOW_H - r());
        if(pos_.x_ < 0 + r()) {
            pos_.x_ = 0 + r();
        }
        if(pos_.y_ < 0 + r() - 1) {
            pos_.y_ = 0 + r();
        }
        if(pos_.x_ > WINDOW_W - r()) {
            pos_.x_ = WINDOW_W - r();
        }
        if(pos_.y_ > WINDOW_H - r()) {
            pos_.y_ = WINDOW_H - r();
        }
        if(is_over_screen == true) {
            is_dash_ = false;
        }
    }
    // ダッシュする
    void setDash()
    {
        dash_pos_    = {pos_.x_ + (r() * 8) * cosf(D2R(dash_angle_)), pos_.y_ + (r() * 8) * sinf(D2R(dash_angle_))};
        is_dash_     = true;
        cool_time_   = 10;
        charge_time_ = 0;
    }
    // ダッシュ可能かどうか
    bool canDash()
    {
        bool is_direct_key_on = isInputDirectionKey();

        if(is_direct_key_on == false)
            return false;

        if(cool_time_ != 0)
            return false;

        if(IsPadRelease(PAD_ID::PAD_A, pad_no_))
            return false;

        return true;
    }
};

class Crown
{
public:
    Crown() { setRandomAngle(); }
    ~Crown() {}

    void update()
    {
        if(king_ != nullptr)
            return;

        // 画面上を縦横無尽に動く
        pos_.x_ += 2.0f * cosf(D2R(move_angle_));
        pos_.y_ += 2.0f * sinf(D2R(move_angle_));
        if(pos_.x_ < 0 + r() || pos_.x_ > WINDOW_W - r() || pos_.y_ < 0 + r() || pos_.y_ > WINDOW_H - r()) {
            setRandomAngle();
        }

        pos_.x_ = std::max(pos_.x_, r());
        pos_.y_ = std::max(pos_.y_, r());
        pos_.x_ = std::min(pos_.x_, WINDOW_W - r());
        pos_.y_ = std::min(pos_.y_, WINDOW_H - r());
    }
    void render()
    {
        if(king_ != nullptr)
            return;

        DrawCircleAA(pos_.x_, pos_.y_, r(), 64, GetColor(255, 215, 0));
    }

    void setKing(const Player* king) { king_ = king; }
    void unsetKing()
    {
        // 現在のプレイヤー位置からランダムな場所に移動させる
        setRandomAngle();
        pos_.x_ = pos_.x_ + (r() + 20) * cosf(D2R(move_angle_));
        pos_.y_ = pos_.y_ + (r() + 20) * sinf(D2R(move_angle_));
        king_   = nullptr;
    }

    // 座標
    const Pos2& pos() const { return pos_; }
    // 半径
    f32 r() const { return 15.0f; }

private:
    Pos2          pos_        = {(f32)(WINDOW_W / 2), (f32)(WINDOW_H / 2)};
    f32           move_angle_ = 0.0f;
    const Player* king_       = nullptr;

    void setRandomAngle() { move_angle_ = (f32)(GetRand(36) * 10); }
};

std::vector<Player*> players;
Crown*               crown = nullptr;

// 当たり判定（座標と半径で）
bool isHit(const Pos2& pos1, f32 r1, const Pos2& pos2, f32 r2)
{
    f32 x_dist = pos1.x_ - pos2.x_;
    f32 y_dist = pos1.y_ - pos2.y_;
    f32 dist   = sqrtf(x_dist * x_dist + y_dist * y_dist);

    return (dist <= r1 + r2);
}

// 当たり判定（プレイヤーと王冠で）
bool isHit(const Player* obj1, const Crown* obj2)
{
    return isHit(obj1->pos(), obj1->r(), obj2->pos(), obj2->r());
}
// 当たり判定（プレイヤーとプレイヤーで）
bool isHit(const Player* obj1, const Player* obj2)
{
    return isHit(obj1->pos(), obj1->r(), obj2->pos(), obj2->r());
}

}    // namespace

bool SceneInputPad::Init()
{
    // グリッド表示をOFF
    ShowGrid(false);

    // 接続されているパッドの数だけプレイヤーを生成
    //  【パッド入力】接続されているパッドの数を取得
    s32 pad_num = GetConnecetdPadNum();
    if(pad_num != -1) {
        // パッドが必ず1台以上接続されていれば、その分プレイヤーを生成
        players.reserve(pad_num);
        for(s32 i = 0; i < pad_num; ++i) {
            u32 u32_pad_no = static_cast<u32>(PAD_NO::PAD_NO1) + i;
            players.emplace_back(new Player(static_cast<PAD_NO>(u32_pad_no)));
        }
    }

    crown = new Crown();

    return true;
}

void SceneInputPad::Update()
{
    for(const auto& player : players) {
        player->update();
    }
    crown->update();

    // 王冠との当たり判定
    for(const auto& player : players) {
        if(isHit(player, crown) == true) {
            crown->setKing(player);
            player->setKing();
        }
    }
    // 他のプレイヤーとの当たり判定
    for(u32 i = 0; i < players.size(); ++i) {
        Player* me = players[i];
        for(u32 j = 0; j < players.size(); ++j) {
            if(i == j)
                continue;

            Player* other = players[j];
            if(isHit(me, other) == true) {
                if(me->isKing()) {
                    me->unsetKing();
                    crown->unsetKing();
                }
                if(other->isKing()) {
                    other->unsetKing();
                    crown->unsetKing();
                }
            }
        }
    }
}

void SceneInputPad::Draw()
{
    for(const auto& player : players) {
        player->render();
    }
    crown->render();

    // パッドが1台も接続されていなければ、画面にメッセージを描画
    //  【パッド入力】パッドの接続台数を取得
    if(GetConnecetdPadNum() == -1) {
        constexpr u32         BOX_W   = 1000;
        constexpr u32         BOX_H   = 250;
        constexpr const char* MESSAGE = "ゲームパッドの接続がありません";

        Pos2 box_pos = {(f32)(WINDOW_W / 2 - BOX_W / 2), (f32)(WINDOW_H / 2 - BOX_H / 2)};
        DrawBoxAA(box_pos.x_, box_pos.y_, box_pos.x_ + BOX_W, box_pos.y_ + BOX_H, BLACK, TRUE);

        s32 font_size = GetFontSize();
        SetFontSize(54);
        u32  font_width = GetDrawFormatStringWidth(MESSAGE);
        Pos2 font_pos   = {(f32)(WINDOW_W / 2 - font_width / 2), (f32)(WINDOW_H / 2 - 54 / 2)};
        DrawFormatStringF(font_pos.x_, font_pos.y_, WHITE, "%s", MESSAGE);
        SetFontSize(font_size);
    }
    else {
        s32 font_size = GetFontSize();
        SetFontSize(16);
        DrawString(0, 0, "左スティックで移動", WHITE);
        DrawString(0, 16, "方向キー＋Aボタンでダッシュ", WHITE);
        SetFontSize(font_size);
    }
}

void SceneInputPad::Exit()
{
    for(auto& player : players) {
        delete player;
        player = nullptr;
    }
    players.clear();

    if(crown != nullptr) {
        delete crown;
        crown = nullptr;
    }
}

void SceneInputPad::GUI()
{
}
