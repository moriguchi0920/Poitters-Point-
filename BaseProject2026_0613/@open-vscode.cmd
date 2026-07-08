@echo off
@setlocal enabledelayedexpansion
chcp 65001 > nul

::----------------------------------------------------
:: プロジェクトおよび .vscode 設定ファイルを生成
::----------------------------------------------------
pushd "%~dp0"
    echo VS Code 用の環境を構築しています...
    call bin\setup_vscode.bat
    call "@make.cmd"
popd

::----------------------------------------------------
:: ワークスペースを開く
:: .code-workspace ファイルに関連付けられたアプリ（VS Code）で起動します
::----------------------------------------------------
if exist "BaseProject2026.code-workspace" (
    echo VS Code を起動しています...
    start "" "BaseProject2026.code-workspace"
) else (
    echo エラー: BaseProject2026.code-workspace が見つかりません。
    pause
)

exit /b 0
