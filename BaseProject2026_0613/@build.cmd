@echo off
@setlocal enabledelayedexpansion
chcp 65001 > nul

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

set ROOT_PATH=%~dp0
pushd "%ROOT_PATH%"

:: プロジェクトファイルの生成
echo [1/2] Generating project files...
call "@make.cmd"
if %errorlevel% neq 0 (
    echo Error: Project generation failed.
    exit /b %errorlevel%
)

:: MSBuildのパスを取得
echo [2/2] Building solution (%CONFIG%)...
set VSWHERE_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "tokens=* usebackq" %%a in (`%VSWHERE_PATH% -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    set MSBUILD_PATH="%%a"
)

if not defined MSBUILD_PATH (
    echo Error: MSBuild.exe not found.
    exit /b 1
)

:: ビルド実行 (BaseProject.slnx または BaseProject.sln)
if exist ".build\BaseProject.slnx" (
    %MSBUILD_PATH% ".build\BaseProject.slnx" /p:Configuration=%CONFIG% /p:Platform=x64 -m
) else if exist ".build\BaseProject.sln" (
    %MSBUILD_PATH% ".build\BaseProject.sln" /p:Configuration=%CONFIG% /p:Platform=x64 -m
) else (
    echo Error: Solution file not found in .build directory.
    exit /b 1
)

if %errorlevel% neq 0 (
    echo Error: Build failed.
    exit /b %errorlevel%
)

echo Build successful.
popd
exit /b 0
