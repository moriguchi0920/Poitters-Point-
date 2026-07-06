@echo off
@setlocal enabledelayedexpansion
chcp 65001 > nul

:: pushd "%~dp0"
:: call "@cleanup.cmd" warm
:: popd

::---------------------------------------------------------------
:: Gitコミットフックを適用
::---------------------------------------------------------------
pushd "%~dp0"

    if exist .git (
        copy bin\pre-commit .git\hooks > nul
    )

popd

::---------------------------------------------------------------
:: clang-format定義ファイルをコピー
::---------------------------------------------------------------
pushd "%~dp0src"

    if not exist .clang-format (
        copy .clang-format-reference .clang-format > nul
        echo .clang-formatファイルを作成しました.
    )

popd

::---------------------------------------------------------------
:: .buildフォルダを作成
:: 環境変数BUILD_PATHを設定した場合は作成先を移動可能。
::---------------------------------------------------------------
pushd "%~dp0"

    :: テンポラリフォルダ用にGUIDを取得
    for /f %%a in ('powershell -command "$([guid]::NewGuid().ToString())"') do (set GUID=%%a)

    :: .buildがなかったら通常作成するためスキップ
    if not exist .build goto build_cleanup_skip

    :: すでに.buildがアクセスできたらスキップ。dirがエラーになったらジャンクションが途切れている
    :: dirでアクセス可能か試す
    dir .build /b > nul 2>&1
    if %errorlevel%  equ 0 (
        goto build_cleanup_skip
    )

    :: 途切れて壊れているジャンクションを一旦解消
    echo .build ジャンクションを再構築します.
    rd /S /Q .build

:build_cleanup_skip

    :: すでにフォルダが存在していたら作成をスキップ
    if exist .build goto skip

    :: カスタムビルドパスの環境変数が存在しなかった場合はスキップ
    if not defined BUILD_PATH goto skip

    echo 環境変数BUILD_PATHが設定されています。%BUILD_PATH%
    echo BUILD_PATH=%BUILD_PATH%

    :: シンボリックリンク接続先のフォルダ生成
    if not exist "%BUILD_PATH%\.build\%GUID%" (
        md "%BUILD_PATH%\.build\%GUID%"
    )

    :: ジャンクション作成 (シンボリックリンクは管理者権限が必要だがジャンクションは不要)
    mklink /j .build "%BUILD_PATH%\.build\%GUID%"

:skip

::---------------------------------------------------------------
:: Visual Studio のインストール状況を取得
::---------------------------------------------------------------
:: vswhere.exe のパス
set VS_INSTALLER_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer
set VSWHERE_PATH=%VS_INSTALLER_PATH%\vswhere.exe
set VS_NAME=vs2026

pushd "%VS_INSTALLER_PATH%"

	for /f "tokens=* usebackq" %%a in (`vswhere.exe -version "[17.0,18.0)" -latest -property installationPath`) do (
	    set "VS2022_PATH=%%a"
	)
	for /f "tokens=* usebackq" %%a in (`vswhere.exe -version "[18.0,19.0)" -latest -property installationPath`) do (
	    set "VS2026_PATH=%%a"
	)

	if "%VS2022_PATH%"=="" (
	    set VS2022_PATH=
	) else (
	    set VS2022_PATH=%VS2022_PATH%\Common7\IDE
	    set VS_NAME=vs2022
	)
	
	if "%VS2026_PATH%"=="" (
	    set VS2026_PATH=
	) else (
	    set VS2026_PATH=%VS2026_PATH%\Common7\IDE
	    set VS_NAME=vs2026
	)

popd

:: 引数でバージョンが指定されている場合は上書き (例: @make.cmd vs2022)
if not "%1"=="" (
    set VS_NAME=%1
)

::---------------------------------------------------------------
:: プロジェクトファイルを生成
::---------------------------------------------------------------
:: 他のバッチファイルからこのバッチがcallされたときにpushdでフォルダ移動すると何故か%~dp0も変化してしまう 
:: 不具合回避としてROOT_PATHに保存する 
set ROOT_PATH=%~dp0

    :: .build フォルダを作成 
    if exist .build goto build_created
        md .build > nul
    :build_created

    :: .build/bin フォルダからbinフォルダを見られるようにする 
    pushd ".build" 
        if exist bin (rd /S /Q bin)
        mklink /j "bin" "%ROOT_PATH%bin" > nul
    popd

    :: WindowsのTemp内にジャンクション作成 (日本語パス対策) 
    mklink /j "%TEMP%\%GUID%" "%ROOT_PATH%" > nul

    :: Temp内からプロジェクト生成
    pushd "%TEMP%\%GUID%\.build"
        copy ..\premake5.lua premake5.lua > nul
        bin\premake5.exe %VS_NAME%
    popd

    if %ERRORLEVEL% neq 0 (
        echo.
        echo エラーが発生しました。Luaスクリプトを確認してください。
        echo.
        echo ==========
        echo "～～/premake5.lua: No such file or directory" と赤い文字で表示される場合はフォルダパスに全角文字が含まれていないか
        echo 配置場所を確認してください。
        echo.
        echo [現在の配置場所]
        cd
        echo.
        echo 全角文字（日本語など）が含まれています。
        echo.
        echo ==========
        echo.
        pause
    )

popd

:finish
    exit /b 0
