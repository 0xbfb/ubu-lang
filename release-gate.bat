@echo off
call test.bat || exit /b 1
call ubu-audit.bat || exit /b 1
call pure-ubu-gate.bat || exit /b 1
echo release-gate PASS
