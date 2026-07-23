@echo off
call build.bat || exit /b 1
build\ucc.exe doctor
