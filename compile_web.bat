@echo off
echo ============================================
echo   SPCMS Web Server - Build Script
echo ============================================
echo.

REM Use MinGW-w64 with POSIX threads (required for httplib)
set "MINGW_BIN=C:\Users\shelk\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
set "GCC=%MINGW_BIN%\gcc.exe"
set "GPP=%MINGW_BIN%\g++.exe"

REM Compile sqlite3.c as C first
echo [1/3] Compiling SQLite3...
"%GCC%" -c -O2 -o sqlite3.o src/sqlite3.c -Iinclude
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] SQLite3 compilation failed!
    pause
    exit /b 1
)

REM Compile all C++ sources
echo [2/3] Compiling C++ sources...
"%GPP%" -std=c++17 -O2 -o spcms_web.exe ^
    web_main.cpp ^
    src/Database.cpp ^
    src/AuthManager.cpp ^
    src/Validator.cpp ^
    src/WebServer.cpp ^
    sqlite3.o ^
    -Iinclude ^
    -lws2_32 -lwsock32 -lpthread
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] C++ compilation failed!
    pause
    exit /b 1
)

REM Cleanup
del sqlite3.o 2>nul

echo [3/3] Build complete!
echo.
echo   Run: spcms_web.exe
echo   Open: http://localhost:8080
echo.
pause
