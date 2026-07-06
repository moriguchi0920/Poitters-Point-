@echo off
setlocal enabledelayedexpansion
chcp 65001 > nul

set VSCODE_DIR=%~dp0..\.vscode
if not exist "%VSCODE_DIR%" (
    mkdir "%VSCODE_DIR%"
)

::-------------------------------------------------------------------------------
:: tasks.json
::-------------------------------------------------------------------------------
(
echo {
echo     "version": "2.0.0",
echo     "tasks": [
echo         {
echo             "label": "Build Project (Debug)",
echo             "type": "shell",
echo             "command": ".\\@build.cmd Debug",
echo             "group": {
echo                 "kind": "build",
echo                 "isDefault": true
echo             },
echo             "problemMatcher": "$msCompile"
echo         },
echo         {
echo             "label": "Build Project (Release)",
echo             "type": "shell",
echo             "command": ".\\@build.cmd Release",
echo             "group": "build",
echo             "problemMatcher": "$msCompile"
echo         },
echo         {
echo             "label": "Build Project (ReleaseLTCG)",
echo             "type": "shell",
echo             "command": ".\\@build.cmd ReleaseLTCG",
echo             "group": "build",
echo             "problemMatcher": "$msCompile"
echo         },
echo         {
echo             "label": "Generate Projects (premake)",
echo             "type": "shell",
echo             "command": ".\\@make.cmd",
echo             "group": "build",
echo             "problemMatcher": []
echo         },
echo         {
echo             "label": "Open in Visual Studio",
echo             "type": "shell",
echo             "command": ".\\@open.cmd",
echo             "group": "none",
echo             "problemMatcher": []
echo         },
echo         {
echo             "label": "Clean Project",
echo             "type": "shell",
echo             "command": ".\\@cleanup.cmd",
echo             "group": "none",
echo             "problemMatcher": []
echo         }
echo     ]
echo }
) > "%VSCODE_DIR%\tasks.json"

::-------------------------------------------------------------------------------
:: launch.json
::-------------------------------------------------------------------------------
(
echo {
echo     "version": "0.2.0",
echo     "configurations": [
echo         {
echo             "name": "(Debug) Build and Run",
echo             "type": "cppvsdbg",
echo             "request": "launch",
echo             "program": "${workspaceFolder}/.build/BaseProject/exe/x64/Debug/BaseProject.exe",
echo             "args": [],
echo             "stopAtEntry": false,
echo             "cwd": "${workspaceFolder}",
echo             "environment": [],
echo             "console": "internalConsole",
echo             "preLaunchTask": "Build Project (Debug)"
echo         },
echo         {
echo             "name": "(Release) Build and Run",
echo             "type": "cppvsdbg",
echo             "request": "launch",
echo             "program": "${workspaceFolder}/.build/BaseProject/exe/x64/Release/BaseProject.exe",
echo             "args": [],
echo             "stopAtEntry": false,
echo             "cwd": "${workspaceFolder}",
echo             "environment": [],
echo             "console": "internalConsole",
echo             "preLaunchTask": "Build Project (Release)"
echo         },
echo         {
echo             "name": "(ReleaseLTCG) Build and Run",
echo             "type": "cppvsdbg",
echo             "request": "launch",
echo             "program": "${workspaceFolder}/.build/BaseProject/exe/x64/ReleaseLTCG/BaseProject.exe",
echo             "args": [],
echo             "stopAtEntry": false,
echo             "cwd": "${workspaceFolder}",
echo             "environment": [],
echo             "console": "internalConsole",
echo             "preLaunchTask": "Build Project (ReleaseLTCG)"
echo         },
echo         {
echo             "name": "(Debug) Run Only",
echo             "type": "cppvsdbg",
echo             "request": "launch",
echo             "program": "${workspaceFolder}/.build/BaseProject/exe/x64/Debug/BaseProject.exe",
echo             "args": [],
echo             "stopAtEntry": false,
echo             "cwd": "${workspaceFolder}",
echo             "environment": [],
echo             "console": "internalConsole"
echo         },
echo         {
echo             "name": "(Release) Run Only",
echo             "type": "cppvsdbg",
echo             "request": "launch",
echo             "program": "${workspaceFolder}/.build/BaseProject/exe/x64/Release/BaseProject.exe",
echo             "args": [],
echo             "stopAtEntry": false,
echo             "cwd": "${workspaceFolder}",
echo             "environment": [],
echo             "console": "internalConsole"
echo         },
echo         {
echo             "name": "(ReleaseLTCG) Run Only",
echo             "type": "cppvsdbg",
echo             "request": "launch",
echo             "program": "${workspaceFolder}/.build/BaseProject/exe/x64/ReleaseLTCG/BaseProject.exe",
echo             "args": [],
echo             "stopAtEntry": false,
echo             "cwd": "${workspaceFolder}",
echo             "environment": [],
echo             "console": "internalConsole"
echo         }
echo     ]
echo }
) > "%VSCODE_DIR%\launch.json"

::-------------------------------------------------------------------------------
:: c_cpp_properties.json
::-------------------------------------------------------------------------------
(
echo {
echo     "configurations": [
echo         {
echo             "name": "Win32",
echo             "includePath": [
echo                 "${workspaceFolder}/src",
echo                 "${workspaceFolder}/dxlib",
echo                 "${workspaceFolder}/opensource/imgui",
echo                 "${workspaceFolder}/opensource",
echo                 "${workspaceFolder}/opensource/cereal/include",
echo                 "${workspaceFolder}/opensource/JoltPhysics",
echo                 "${workspaceFolder}/src/Sample",
echo                 "${workspaceFolder}/opensource/hlslpp/include"
echo             ],
echo             "defines": [
echo                 "_DEBUG",
echo                 "_MBCS",
echo                 "WIN32",
echo                 "_WINDOWS",
echo                 "JPH_DEBUG_RENDERER=1",
echo                 "_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING",
echo                 "_HAS_CHAR8_T=0"
echo             ],
echo             "compilerArgs": [
echo                 "/Zc:char8_t-",
echo                 "/permissive-"
echo             ],
echo             "windowsSdkVersion": "10.0.22621.0",
echo             "compilerPath": "cl.exe",
echo             "cStandard": "c17",
echo             "cppStandard": "c++20",
echo             "intelliSenseMode": "windows-msvc-x64",
echo             "forcedInclude": [
echo                 "${workspaceFolder}/src/Precompile.h"
echo             ]
echo         }
echo     ],
echo     "version": 4
echo }
) > "%VSCODE_DIR%\c_cpp_properties.json"

echo .vscode 設定ファイルを再構築しました。

::-------------------------------------------------------------------------------
:: BaseProject2026.code-workspace
::-------------------------------------------------------------------------------
set WORKSPACE_FILE=%~dp0..\BaseProject2026.code-workspace
if not exist "%WORKSPACE_FILE%" (
    (
    echo {
    echo 	"folders": [
    echo 		{
    echo 			"path": "."
    echo 		}
    echo 	],
    echo 	"settings": {
    echo 		"files.associations": {
    echo 			"*.h": "cpp",
    echo 			"*.inl": "cpp"
    echo 		},
    echo 		"C_Cpp.default.cppStandard": "c++20",
    echo 		"C_Cpp.default.intelliSenseMode": "windows-msvc-x64"
    echo 	}
    echo }
    ) > "%WORKSPACE_FILE%"
    echo ワークスペースファイルを再構築しました。
)

exit /b 0
