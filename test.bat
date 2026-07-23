@echo off
call build.bat || exit /b 1
py scripts\run_tests.py
