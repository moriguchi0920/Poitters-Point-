//---------------------------------------------------------------------------
//! @file   PoittersPoint_Camera.cpp
//! @brief  PoittersPoint_Camera
//!
//! ■ 1フレームの処理の流れ (PostPhysics)
//!   1. 生存者の足元座標を集める ............ CollectSurvivorPositions()
//!   2. 生存者が作る図形の中心点を求める .... CalcShapeCenter()      → desired_target_
//!   3. 注視点を目標へ少しずつ近づける ...... SmoothFactor() + lerp → current_target_
//!   4. 全員が映る距離を求める .............. CalcRequiredDistance() → desired_distance_
//!   5. 距離を目標へ少しずつ近づける ........ SmoothFactor()        → current_distance_
//!   6. 注視点と距離からカメラ位置を決める .. ApplyToCamera()
//!
//! ■ カメラの置き方(横から見た図)
//!
//!        カメラ ●
//!                 ＼   ← 前方向ベクトル fwd (見下ろし角度 pitch で斜め下を向く)
//!                   ＼
//!       distance      ＼
//!                       ＼  pitch
//!                         ● ─────────── 地面
//!                       注視点 target
//!
//!   カメラ位置 = 注視点 - fwd × 距離
//!   つまり「注視点から前方向の逆向きに、距離だけ下がった場所」にカメラを置く。
//!   向き(pitch, yaw)は固定なので、変わるのは「注視点」と「距離」の2つだけ。
//---------------------------------------------------------------------------
#include "PoittersPoint_Camera.h"
#include "PoittersPoint_Player.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/ImGui.h>
#include <System/Component/ComponentModel.h>

#include <algorithm>
#include <cmath>

namespace PoittersPoint {
// namespace PoittersPoint

namespace {

//! @brief フレームレートに依存しない補間係数を求める
//! @param speed 追従速度(1秒あたり)
//! @param dt    経過時間(秒)
//! @return 0～1 の補間係数(lerpの第3引数に使う)
//!
//! 単純に lerp(現在, 目標, 0.1f) を毎フレーム行うと、
//! 60fpsでは1秒に60回、30fpsでは30回しか近づかないため、FPSで追従の速さが変わってしまう。
//!
//! そこで「1秒間に speed の割合で指数的に近づく」という動きを考える。
//!   残りの差 = 最初の差 × e^(-speed × 経過時間)
//! 1フレーム(dt秒)の間に残る割合は e^(-speed×dt) なので、
//! 1フレームで縮める割合(=補間係数)は 1 - e^(-speed×dt) になる。
//!
//! 例) speed = 2.5, dt = 1/60 秒 → 1 - e^(-0.0417) ≒ 0.041 (1フレームで差の約4%を縮める)
//!     speed = 2.5, dt = 1/30 秒 → 1 - e^(-0.0833) ≒ 0.080 (フレームが半分なので1回で約8%)
//!     → どちらも1秒後には差が e^(-2.5) ≒ 8% まで縮み、FPSに関係なく同じ動きになる。
float SmoothFactor(float speed, float dt)
{
    return 1.0f - expf(-speed * dt);
}

//! @brief XZ平面上の外積(凸包計算用)
//! @param o 基準点
//! @param a 点A
//! @param b 点B
//! @return ベクトル OA と OB の2次元外積
//!
//! 真上(Y軸)から見下ろしたXZ平面で、O→A→B と進んだときの曲がり方を判定する。
//!   (a - o) × (b - o) = (a.x - o.x)(b.z - o.z) - (a.z - o.z)(b.x - o.x)
//!   値 > 0 : 左に曲がる(反時計回り)
//!   値 < 0 : 右に曲がる(時計回り)
//!   値 = 0 : 一直線上
//! ※値の大きさは OA と OB で作る平行四辺形の面積になる(面積計算でも同じ考え方を使う)
float Cross2D(const float3& o, const float3& a, const float3& b)
{
    return (a.x - o.x) * (b.z - o.z) - (a.z - o.z) * (b.x - o.x);
}

//! @brief XZ平面上の凸包を求める(Andrewのモノトーンチェーン法)
//! @param pts 点の配列(コピーを受け取り、関数内で並び替える)
//! @return 反時計回りに並んだ凸包の頂点
//!
//! 凸包 = 全ての点を輪ゴムで囲んだときの形。
//! 4人以上いるとき、内側にいるキャラクターは図形の頂点にならない(輪ゴムに触れない)。
//!
//!       ●A                ●A
//!      ●E   ●B    →     ／  ＼
//!   ●D     ●C          ●D ─ ●C ─ ●B   (Eは内側なので頂点にならない)
//!
//! 手順:
//!   1. 点をX座標の小さい順(同じならZの小さい順)に並べる
//!   2. 左から右へ進みながら「下側の輪郭」を作る
//!   3. 右から左へ戻りながら「上側の輪郭」を作る
//!   各段階で、直前の2点と新しい点が「右に曲がる(または一直線)」なら
//!   直前の点は内側にへこんでいるので取り除く(--k)。
std::vector<float3> ConvexHullXZ(std::vector<float3> pts)
{
    // 3点未満は図形(面)にならないのでそのまま返す
    if(pts.size() < 3) {
        return pts;
    }
    // 手順1: X座標の昇順、同じXならZ座標の昇順に並び替え
    std::sort(pts.begin(), pts.end(), [](const float3& a, const float3& b) { return (a.x < b.x) || (a.x == b.x && a.z < b.z); });

    // 下側と上側の輪郭を合わせても最大で点の数×2を超えない
    std::vector<float3> hull(pts.size() * 2);
    size_t              k = 0;    // hullに入っている有効な頂点の数

    // 手順2: 下側の輪郭(左端 → 右端)
    for(size_t i = 0; i < pts.size(); ++i) {
        // 直前の2点(hull[k-2] → hull[k-1])から新しい点 pts[i] へ進むとき、
        // 左に曲がらない(外積 <= 0)なら hull[k-1] はへこんでいるので捨てる
        while(k >= 2 && Cross2D(hull[k - 2], hull[k - 1], pts[i]) <= 0.0f) {
            --k;
        }
        hull[k++] = pts[i];
    }

    // 手順3: 上側の輪郭(右端 → 左端)
    // t = 下側の輪郭の頂点数 + 1。上側の処理では下側で確定した頂点を消さないようにするための下限
    for(size_t i = pts.size() - 1, t = k + 1; i > 0; --i) {
        while(k >= t && Cross2D(hull[k - 2], hull[k - 1], pts[i - 1]) <= 0.0f) {
            --k;
        }
        hull[k++] = pts[i - 1];
    }

    // 最後に追加した点は始点(左端の点)と同じなので1つ減らす
    hull.resize(k - 1);
    return hull;
}

}    // namespace

//! @brief 初期化
//! @return 初期化終了
bool Camera::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();

    SetName("Camera");

    AddComponent<ComponentCamera>();    //カメラコンポーネントを付ける
    if(auto c = GetComponent<ComponentCamera>()) {
        c->SetPerspective(fovy_deg_, aspect_ratio_);
        // 生存者がまだ揃っていない間のステージ全体を見渡す初期位置
        c->SetPositionAndTarget({0, 80, 205}, {0, 0, -60});
    }

    // 最初に生存者が見つかったフレームで、補間せずに一気に位置を合わせるためのフラグ
    initialized_ = false;
    return true;
}

//! @brief 物理処理後の更新
//!
//! Update()の時点ではまだ物理(当たり判定・重力)で位置が動く前なので、
//! 全キャラクターの位置が確定した PostPhysics() でカメラを動かす。
//! (ComponentCameraは PostUpdate() で行列を作るため、ここで設定した値がそのフレームに反映される)
void Camera::PostPhysics()
{
    Super::PostPhysics();

    //-----------------------------------------------------------
    // 1. 生存者を集めて、図形の中心点(目標の注視点)を求める
    //-----------------------------------------------------------
    survivors_ = CollectSurvivorPositions();

    // 生存者がいなければ直前の状態を維持する
    if(!survivors_.empty()) {
        desired_target_ = CalcShapeCenter(survivors_);
    }

    // 1フレームの経過時間(秒)
    // 読み込みなどで一瞬止まると dt が大きくなり、補間で一気に飛んでしまうため上限を 0.1秒 にする
    float dt = GetDeltaTime();
    dt       = std::clamp(dt, 0.0f, 0.1f);    // ヒッチ時に飛びすぎないように制限

    if(!initialized_) {
        //-----------------------------------------------------------
        // 初回: 補間せずに一気に目標へ合わせる
        // (初期位置からゆっくり移動してくると開始直後に全員が映らない時間ができるため)
        //-----------------------------------------------------------
        if(survivors_.empty()) {
            return;    // 誰もいない間は初期位置のまま
        }
        current_target_   = desired_target_;
        desired_distance_ = CalcRequiredDistance(survivors_, current_target_);
        current_distance_ = desired_distance_;
        initialized_      = true;
    }
    else {
        //-----------------------------------------------------------
        // 2. 注視点を滑らかに追従
        //    lerp(a, b, t) = a + (b - a) × t
        //    t は SmoothFactor() で求めた「このフレームで差を何割縮めるか」
        //-----------------------------------------------------------
        current_target_ = lerp(current_target_, desired_target_, SmoothFactor(target_follow_speed_, dt));

        //-----------------------------------------------------------
        // 3. 全員が映る距離を求める
        //    目標の注視点(desired)ではなく、実際に今見ている注視点(current)を基準に計算する。
        //    注視点が移動途中でも、その位置から全員が映る距離になるので映り切らないことがない。
        //-----------------------------------------------------------
        if(!survivors_.empty()) {
            desired_distance_ = CalcRequiredDistance(survivors_, current_target_);
        }

        //-----------------------------------------------------------
        // 4. 距離を滑らかに追従
        //    引くとき(目標の方が遠い) : 速く追従 → 遅れると画面外にはみ出すため
        //    寄るとき(目標の方が近い) : ゆっくり追従 → 急に寄ると落ち着かないため
        //    式は lerp と同じ: 現在 + (目標 - 現在) × 補間係数
        //-----------------------------------------------------------
        float speed       = (desired_distance_ > current_distance_) ? zoom_out_speed_ : zoom_in_speed_;
        current_distance_ = current_distance_ + (desired_distance_ - current_distance_) * SmoothFactor(speed, dt);
    }

    // 5. 注視点と距離からカメラ位置を決めてカメラコンポーネントへ設定
    ApplyToCamera();
}

//! @brief 現在の生存者の位置を集める
//! @return 生存者の足元座標(Playerオブジェクトの位置)の配列
std::vector<float3> Camera::CollectSurvivorPositions() const
{
    std::vector<float3> result;

    // 操作プレイヤーもCPUも同じPlayerクラスなので、Playerを全て取得すれば参加者全員になる
    for(auto& player : Scene::Object::GetArray<Player>()) {
        if(!player) {
            continue;
        }
        // 脱落した(HPが尽きて死亡処理が始まった)キャラクターは除外
        if(!player->IsAlive()) {
            continue;
        }
        // ※HPが0でも持ち上げ・投げの途中などは脱落処理を待つため、HPではなくIsAlive()で判定する
        float3 pos = player->GetTranslate();
        // ステージ外へ落下したキャラクターは除外
        // (除外しないと落ちていくキャラを追ってカメラが下へ引っ張られてしまう)
        if(pos.y < fall_out_height_) {
            continue;
        }
        result.push_back(pos);
    }
    return result;
}

//! @brief 生存者が作る図形の中心点を求める
//! @param points 生存者の座標(1つ以上)
//! @return 図形の中心点
//!
//!   1人 : その人の位置                 ●
//!   2人 : 2人を結ぶ線分の中点          ●──×──●
//!   3人 : 三角形の重心                 3頂点の平均
//!   4人以上 : 全員を囲む多角形の重心   面積で重みをつけた平均(下で説明)
float3 Camera::CalcShapeCenter(const std::vector<float3>& points)
{
    //-----------------------------------------------------------
    // 頂点の平均 = (p1 + p2 + ... + pn) / n
    // 1人:その位置 / 2人:中点 / 3人:三角形の重心 とそれぞれ一致する
    //-----------------------------------------------------------
    float3 mean = {0, 0, 0};
    for(auto& p : points) {
        mean += p;
    }
    mean /= (float)points.size();

    if(points.size() < 3) {
        return mean;
    }

    //-----------------------------------------------------------
    // 3人以上は生存者を囲む多角形(凸包)の面積重心を中心点とする
    //
    // 4人以上で頂点の平均を使うと、片側に人が固まっているだけで中心が寄ってしまう。
    //   例) 左に3人が固まり、右に1人 → 平均は左寄りになり、右の1人が画面端へ追いやられる
    // 「図形の中心」としては、形そのものの重心(板を切り抜いたときに釣り合う点)を使う。
    // (3人のときは三角形の重心 = 頂点の平均 なので、どちらで計算しても同じ結果になる)
    //-----------------------------------------------------------
    auto hull = ConvexHullXZ(points);
    if(hull.size() < 3) {
        return mean;    // 一直線に並んでいる場合(面積0の図形なので平均を使う)
    }

    //-----------------------------------------------------------
    // 多角形の面積重心(靴ひも公式の応用)
    //
    // 多角形を「原点と隣り合う2頂点 a, b で作る三角形」に分割して考える。
    //   その三角形の面積の2倍   : w = a.x × b.z - b.x × a.z  (原点基準の2次元外積)
    //   その三角形の重心        : (原点 + a + b) / 3 = (a + b) / 3
    //
    // 全ての三角形について「重心 × 面積」を足し、合計面積で割ると多角形全体の重心になる。
    //   重心 = Σ{ (a + b) / 3 × w/2 } / Σ{ w/2 }
    //        = Σ{ (a + b) × w } / (3 × Σw)
    //
    // ※凸包の外側にはみ出す三角形の面積は w が負になって打ち消されるため、
    //   原点が多角形の外にあっても正しく計算できる。
    //-----------------------------------------------------------
    float  area2 = 0.0f;    // 面積の2倍 (Σw)
    float3 c     = {0, 0, 0};
    for(size_t i = 0; i < hull.size(); ++i) {
        const float3& a  = hull[i];
        const float3& b  = hull[(i + 1) % hull.size()];    // 次の頂点(最後の頂点の次は最初に戻る)
        float         w  = a.x * b.z - b.x * a.z;          // 三角形(原点, a, b)の面積の2倍
        area2           += w;
        c.x             += (a.x + b.x) * w;    // Σ{ (a + b) × w } のX成分
        c.z             += (a.z + b.z) * w;    // Σ{ (a + b) × w } のZ成分
    }
    // 面積がほぼ0(ほぼ一直線)の場合は割り算が不安定になるので平均を使う
    if(fabsf(area2) < 1.0e-3f) {
        return mean;
    }
    // Σ{ (a + b) × w } / (3 × Σw)
    c.x /= (3.0f * area2);
    c.z /= (3.0f * area2);
    c.y  = mean.y;    // 図形はXZ平面(真上から見た形)で考えているので、高さは平均を使う
    return c;
}

//! @brief カメラの前方向
//! @return 注視点へ向かう単位ベクトル
//!
//! 見下ろし角度(pitch)と水平角度(yaw)から、球面座標の考え方で向きを作る。
//!   ・下向き成分(Y)   : -sin(pitch)   … pitchが大きいほど真下を向く
//!   ・水平成分の長さ  :  cos(pitch)   … pitchが大きいほど水平方向は短くなる
//!   ・水平成分の向き  : yawで回転させる(yaw=0 のとき -Z方向)
//!       X = -sin(yaw) × cos(pitch)
//!       Z = -cos(yaw) × cos(pitch)
//! sin² + cos² = 1 なので、このベクトルの長さは最初から1になっている。
float3 Camera::GetForward() const
{
    float pitch = pitch_deg_ * DegToRad;
    float yaw   = yaw_deg_ * DegToRad;
    // yaw=0 のとき +Z 側から -Z 方向を見下ろす
    return float3(-sinf(yaw) * cosf(pitch), -sinf(pitch), -cosf(yaw) * cosf(pitch));
}

//! @brief 全員を画面に収めるために必要なカメラ距離を求める
//! @param points 生存者の足元座標
//! @param target カメラの注視点
//! @return 必要なカメラ距離(min_distance_～max_distance_の範囲)
//!
//! ■ 考え方
//! 判定したい点を「カメラから見た座標(右 x, 上 y, 奥 z)」に直すと、
//! 透視投影では奥行き d の位置に映る範囲は
//!   縦: 中心から ±d × tan(画角/2)
//!   横: 中心から ±d × tan(画角/2) × アスペクト比
//! になる(遠いほど広く映る)。
//!
//!         カメラ
//!           ●
//!          ╱│╲           画角の半分の角度で広がる
//!         ╱ │ ╲
//!        ╱  │d ╲         奥行き d の位置で映る幅の半分 = d × tan(画角/2)
//!       ╱   │   ╲
//!      ├────┼────┤
//!       映る範囲
//!
//! よって点が画面内に映る条件は
//!   |x| <= d × tan_h   かつ   |y| <= d × tan_v
//! つまり   d >= |x| / tan_h   かつ   d >= |y| / tan_v
//!
//! 点の奥行き d は「カメラ距離 distance + 注視点から見た奥方向のずれ z」なので
//!   distance + z >= max(|x| / tan_h, |y| / tan_v)
//!   distance     >= max(|x| / tan_h, |y| / tan_v) - z
//! これを全ての判定点で満たす最大の distance が答えになる。
float Camera::CalcRequiredDistance(const std::vector<float3>& points, const float3& target) const
{
    //-----------------------------------------------------------
    // カメラの向きを表す3本の軸(どれも長さ1で、互いに直角)
    //   fwd   : 奥方向(カメラが見ている向き)
    //   right : 画面の右方向(水平。yawだけで決まる)
    //   up    : 画面の上方向(fwdとrightの両方に直角な向き = 外積で求める)
    //-----------------------------------------------------------
    float3 fwd   = normalize(GetForward());
    float  yaw   = yaw_deg_ * DegToRad;
    float3 right = normalize(float3(cosf(yaw), 0.0f, -sinf(yaw)));    // fwdの水平成分を90度回した向き
    float3 up    = normalize(cross(fwd, right));

    //-----------------------------------------------------------
    // 奥行き1あたりに映る範囲の半分(上の図の tan(画角/2))
    // screen_margin_ を掛けて画面を少し狭いものとして計算し、画面端ぴったりではなく内側に収める
    //   例) screen_margin_ = 0.8 → 画面の中央80%の範囲に収める
    //-----------------------------------------------------------
    float tan_v = tanf(fovy_deg_ * 0.5f * DegToRad) * screen_margin_;    // 縦方向
    float tan_h = tan_v * aspect_ratio_;                                 // 横方向(画面が横長な分だけ広い)

    // どれだけ寄っても min_distance_ より近づかない
    float required = min_distance_;

    for(auto& p : points) {
        //-----------------------------------------------------------
        // 足元・頭・体の左右を判定点にする
        // 足元の1点だけだと、頭や体の端が画面外にはみ出すことがあるため
        //
        //   ●─●─●  ← 頭の高さ (左端・中心・右端)
        //   │  │  │
        //   ●─●─●  ← 足元     (左端・中心・右端)
        //-----------------------------------------------------------
        const float3 samples[] = {
            p,                                                        // 足元
            p + float3(0, body_height_, 0),                           // 頭
            p + right * body_radius_,                                 // 足元の右端
            p - right * body_radius_,                                 // 足元の左端
            p + right * body_radius_ + float3(0, body_height_, 0),    // 頭の右端
            p - right * body_radius_ + float3(0, body_height_, 0),    // 頭の左端
        };
        for(auto& s : samples) {
            //-----------------------------------------------------------
            // 注視点からの相対位置を、カメラの3本の軸にそれぞれ投影する
            // (単位ベクトルとの内積 = その方向の成分の長さ)
            //-----------------------------------------------------------
            float3 rel = s - target;
            float  x   = fabsf(dot(rel, right));    // 画面中心から左右にどれだけ離れているか
            float  y   = fabsf(dot(rel, up));       // 画面中心から上下にどれだけ離れているか
            float  z   = dot(rel, fwd);             // 注視点より奥ならプラス、手前ならマイナス

            //-----------------------------------------------------------
            // この点を映すのに必要な距離
            //   distance >= max(|x| / tan_h, |y| / tan_v) - z
            // 奥にある点(z > 0)はもともとカメラから遠いので、その分だけ必要な距離が短くて済む
            //-----------------------------------------------------------
            float need = std::max(x / tan_h, y / tan_v) - z;
            required   = std::max(required, need);    // 全ての点の中で一番厳しい条件を採用
        }
    }
    // 離れすぎてキャラクターが豆粒にならないよう最大距離で制限
    return std::clamp(required, min_distance_, max_distance_);
}

//! @brief 現在の注視点・距離をカメラコンポーネントへ反映
void Camera::ApplyToCamera()
{
    if(auto c = GetComponent<ComponentCamera>()) {
        // カメラ位置 = 注視点 - 前方向 × 距離
        // (注視点から、見ている向きと逆方向へ距離ぶん下がった場所)
        float3 position = current_target_ - normalize(GetForward()) * current_distance_;
        c->SetPerspective(fovy_deg_, aspect_ratio_);    // GUIで画角を変えたときにも反映されるよう毎フレーム設定
        c->SetPositionAndTarget(position, current_target_);
    }
}

//! @brief 描画(デバッグ表示)
//!   黄色の線 : 生存者が作る図形(4人以上は凸包)
//!   緑の十字 : 図形の中心点(目標の注視点)
//!   赤の十字 : 現在の注視点(緑の十字を滑らかに追いかける)
void Camera::Draw()
{
    Super::Draw();

    if(!debug_draw_ || survivors_.empty()) {
        return;
    }

    const unsigned int shape_color  = GetColor(255, 255, 0);
    const unsigned int target_color = GetColor(255, 0, 0);
    const unsigned int center_color = GetColor(0, 255, 0);

    // 生存者が作る図形
    // (地面に埋もれて見えなくならないよう、少し(1.0)上げて描画する)
    auto shape = (survivors_.size() >= 3) ? ConvexHullXZ(survivors_) : survivors_;
    for(size_t i = 0; i < shape.size(); ++i) {
        const float3& a = shape[i];
        const float3& b = shape[(i + 1) % shape.size()];    // 最後の頂点は最初の頂点と結んで図形を閉じる
        DrawLine3D(VGet(a.x, a.y + 1.0f, a.z), VGet(b.x, b.y + 1.0f, b.z), shape_color);
    }

    // 十字マーカー(X軸・Z軸・Y軸の3本の線)
    auto drawCross = [](const float3& p, float size, unsigned int color) {
        DrawLine3D(VGet(p.x - size, p.y + 1.0f, p.z), VGet(p.x + size, p.y + 1.0f, p.z), color);
        DrawLine3D(VGet(p.x, p.y + 1.0f, p.z - size), VGet(p.x, p.y + 1.0f, p.z + size), color);
        DrawLine3D(VGet(p.x, p.y + 1.0f - size, p.z), VGet(p.x, p.y + 1.0f + size, p.z), color);
    };
    drawCross(desired_target_, 6.0f, center_color);    // 図形の中心点(目標)
    drawCross(current_target_, 4.0f, target_color);    // 現在の注視点(補間中)
}

//! @brief GUI表示
//! 実行中に値を調整して、気に入った値をヘッダの初期値へ書き戻す使い方を想定
void Camera::GUI()
{
    Super::GUI();

    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
        // 現在の状態(確認用)
        ImGui::Text(u8"生存者数: %d", (int)survivors_.size());
        ImGui::Text(u8"注視点: (%.1f, %.1f, %.1f)", current_target_.x, current_target_.y, current_target_.z);
        ImGui::Text(u8"距離: %.1f (目標 %.1f)", current_distance_, desired_distance_);
        ImGui::Checkbox(u8"図形と中心点を表示", &debug_draw_);
        // 調整用パラメーター
        ImGui::DragFloat(u8"画角(縦)", &fovy_deg_, 0.1f, 10.0f, 120.0f, "%.1f");
        ImGui::DragFloat(u8"見下ろし角度", &pitch_deg_, 0.1f, 5.0f, 89.0f, "%.1f");
        ImGui::DragFloat(u8"水平角度", &yaw_deg_, 0.5f, -180.0f, 180.0f, "%.1f");
        ImGui::DragFloat(u8"画面内に収める割合", &screen_margin_, 0.01f, 0.3f, 1.0f, "%.2f");
        ImGui::DragFloat(u8"最小距離", &min_distance_, 1.0f, 10.0f, 1000.0f, "%.1f");
        ImGui::DragFloat(u8"最大距離", &max_distance_, 1.0f, 10.0f, 2000.0f, "%.1f");
        ImGui::DragFloat(u8"注視点の追従速度", &target_follow_speed_, 0.05f, 0.1f, 20.0f, "%.2f");
        ImGui::DragFloat(u8"寄る速度", &zoom_in_speed_, 0.05f, 0.1f, 20.0f, "%.2f");
        ImGui::DragFloat(u8"引く速度", &zoom_out_speed_, 0.05f, 0.1f, 20.0f, "%.2f");
    }
    ImGui::End();
}

}    // namespace PoittersPoint
