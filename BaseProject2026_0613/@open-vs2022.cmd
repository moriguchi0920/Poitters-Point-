@echo off
@setlocal enabledelayedexpansion
chcp 65001 > nul

::----------------------------------------------------
:: プロジェクトファイルを生成
::----------------------------------------------------
pushd "%~dp0"
call "@make.cmd" vs2022
call "@code_format.cmd" User
popd

::---------------------------------------------------------------
:: Visual Studio のインストール状況を取得
::---------------------------------------------------------------
:: vswhere.exe のパス
set VS_INSTALLER_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer
set VSWHERE_PATH=%VS_INSTALLER_PATH%\vswhere.exe
set VS_NAME=vs2022

pushd "%VS_INSTALLER_PATH%"

	for /f "tokens=* usebackq" %%a in (`vswhere.exe -version "[17.0,18.0)" -latest -property installationPath`) do (
	    set "VS2022_PATH=%%a"
	)
	rem for /f "tokens=* usebackq" %%a in (`vswhere.exe -version "[18.0,19.0)" -latest -property installationPath`) do (
	rem     set "VS2026_PATH=%%a"
	rem )

	if "%VS2022_PATH%"=="" (
	    set VS2022_PATH=
	) else (
	    set VS2022_PATH=%VS2022_PATH%\Common7\IDE
	    set VS_NAME=vs2022
	)
	
	rem if "%VS2026_PATH%"=="" (
	rem     set VS2026_PATH=
	rem ) else (
	rem     set VS2026_PATH=%VS2026_PATH%\Common7\IDE
	rem     set VS_NAME=vs2026
	rem )

popd

::----------------------------------------------------
:: ソリューションファイルを検索
:: 検索で見つけることでファイル名が変化しても対応可能
::----------------------------------------------------
cd "%~dp0\.build"
setlocal enabledelayedexpansion

for /f "usebackq" %%i in (`dir /b *.sln`) do set SOLUTION_FILE=%%i
if not exist "%SOLUTION_FILE%" (
	for /f "usebackq" %%i in (`dir /b *.slnx`) do set SOLUTION_FILE=%%i
)

if not exist "%SOLUTION_FILE%" (
	echo プロジェクトファイルが見つかりません。@make.cmdを実行してください。
	goto error
)

:: ソリューションを起動
if "%VS_NAME%"=="vs2026" (
	start /min "" "%VS2026_PATH%\devenv.exe" "%SOLUTION_FILE%"
) else (
	start /min "" "%VS2022_PATH%\devenv.exe" "%SOLUTION_FILE%"
)

exit

:error
pause
