@echo off
setlocal EnableExtensions

pushd "%~dp0..\.."
if errorlevel 1 (
    echo Failed to enter repository root.
    exit /b 1
)

set "DOCKER_CONFIG=%CD%\.docker"
if not exist "%DOCKER_CONFIG%" mkdir "%DOCKER_CONFIG%"

where docker >nul 2>nul
if errorlevel 1 (
    echo Docker is required to run this demo.
    popd
    exit /b 1
)

if exist "C:\Program Files\Docker\Docker\frontend\Docker Desktop.exe" (
    start "" "C:\Program Files\Docker\Docker\frontend\Docker Desktop.exe"
)

set "DOCKER_READY="
for /L %%I in (1,1,60) do (
    docker info >nul 2>nul
    if not errorlevel 1 set "DOCKER_READY=1"
    if defined DOCKER_READY goto :docker_ready
    timeout /t 2 /nobreak >nul
)

:docker_ready
if not defined DOCKER_READY (
    echo Docker Desktop or the Docker engine must be running.
    popd
    exit /b 1
)

docker compose run --rm --build demo sh scripts/demo/demo_scenario_inner.sh %*
set "EXIT_CODE=%ERRORLEVEL%"
popd
exit /b %EXIT_CODE%
