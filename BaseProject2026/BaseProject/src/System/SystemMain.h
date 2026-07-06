//---------------------------------------------------------------------------
//!	@file	SystemMain.h
//! @brief	システムメイン
//---------------------------------------------------------------------------
#pragma once

//--------------------------------------------------------------
//!	@name	前方宣言
//--------------------------------------------------------------
//!@{

class Texture;

//!@}
//--------------------------------------------------------------
//!	@name	システム関数
//--------------------------------------------------------------
//@{

//	初期化
void SystemInit();

//	更新
void SystemUpdate();

//	描画
void SystemDraw();

//	終了
void SystemExit();

//	フレームの開始
void SystemBeginFrame();

//	フレームの終了
void SystemEndFrame();

//@}
//--------------------------------------------------------------
//!	@name	設定/参照
//--------------------------------------------------------------
//@{

//	パフォーマンスカウンターを取得 (単位:μsec)
u64 GetPerformanceCounterMicroSec();

//	経過時間計測をリセット
void ResetDeltaTime();

// 経過した時間を取得
f32 GetDeltaTime();
// これを掛けると60FPSと同じ速度になる
f32 GetDeltaTime60();
// ツールなどで使用する際強制的にFPSを変更する
void SetDeltaTime(f32);

//	グリッドの表示をON/OFFします
//! @param  [in]    active  true:表示する false:非表示
void ShowGrid(bool active);

//! @brief メニュー表示
//! @return true:表示する false:非表示
bool IsShowMenu();

//! @brief デバッグ表示
//! @return true:表示する false:非表示
bool IsShowDebug();

//! @brief GUI表示
//! @return true:表示する false:非表示
bool IsShowGUI();

//! @brief FPS表示
//! @return true:表示する false:非表示
bool IsShowFPS();

//! @brief Grid表示
//! @return true:表示する false:非表示
bool IsShowGrid();

//! RenderTarget HDRバッファを取得
Texture* GetHdrBuffer();

//@}
