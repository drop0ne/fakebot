@echo off
setlocal

where cl >nul 2>nul
if errorlevel 1 (
    echo Microsoft C++ compiler not found in PATH.
    echo Open a Developer Command Prompt for Visual Studio and run this script again.
    exit /b 1
)

if not exist build mkdir build

cl /std:c++20 /EHsc /W4 /I src ^
    src\main.cpp ^
    src\ChatBot.cpp ^
    src\RuleLoader.cpp ^
    src\TextUtils.cpp ^
    /Fe:build\FakeBot.exe

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build complete: build\FakeBot.exe
