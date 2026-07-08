:: コードメトリクス解析
@echo off
@setlocal enabledelayedexpansion
chcp 65001 > nul

cd /d "%~dp0"

:: テンポラリフォルダ用にGUIDを取得
for /f %%a in ('powershell -command "$([guid]::NewGuid().ToString())"') do (set GUID=%%a)

::----------------------------------------------------------------------------
:: コードメトリクス計測
::----------------------------------------------------------------------------
mklink /j  "%TEMP%\BaseProject_%GUID%" "." > nul
pushd "%TEMP%\BaseProject_%GUID%"
    dir /b /s src\*.cpp src\*.c | bin\cccc -
popd

rd /q "%TEMP%\BaseProject_%GUID%"

::----------------------------------------------------------------------------
:: ブラウザで結果を開く
::----------------------------------------------------------------------------
start .cccc\cccc.html

exit /b 0
