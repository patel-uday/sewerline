@echo off
set "GAME_DIR=%~dp0release"
set "GAME_EXE=%GAME_DIR%\sewerline.exe"
if not exist "%GAME_EXE%" (
  echo SEWERLINE has not been built yet.
  echo Run build-release.ps1 once, then double-click PLAYTEST.bat again.
  pause
  exit /b 1
)
start "SEWERLINE" /D "%GAME_DIR%" "%GAME_EXE%"
