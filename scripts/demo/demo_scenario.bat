@echo off
setlocal EnableExtensions

pushd "%~dp0..\.."
if errorlevel 1 (
    echo Failed to enter repository root.
    exit /b 1
)

where sh >nul 2>nul
if errorlevel 1 (
    echo This demo launcher requires sh.exe on PATH.
    echo Please run it from Git Bash, MSYS2, or WSL.
    popd
    exit /b 1
)

sh scripts/demo/demo_scenario.sh %*
set "EXIT_CODE=%ERRORLEVEL%"
popd
exit /b %EXIT_CODE%
