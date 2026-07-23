@echo off
if not exist build mkdir build
gcc -std=c11 -O2 -Wall -Wextra src\ucc0\main.c -o build\ucc.exe
if errorlevel 1 exit /b 1
echo Built build\ucc.exe
