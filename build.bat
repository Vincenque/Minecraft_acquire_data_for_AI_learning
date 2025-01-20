@echo off
if exist "%~dp0bin\program.exe" del /q "%~dp0bin\program.exe"
if not exist "%~dp0bin" mkdir "%~dp0bin"
gcc -I"%~dp0headers" -LC:/MinGW/lib "%~dp0source\main.c" -o "%~dp0bin\program.exe" -lmingw32