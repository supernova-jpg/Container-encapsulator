@echo off
echo Building Pro Muxer...
echo.

REM Check if qmake is available
where qmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: qmake not found in PATH
    echo Please ensure Qt is installed and qmake is in your PATH
    echo Example: Add C:\Qt\6.x.x\mingw_64\bin to your PATH
    pause
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Clean previous build
if exist Makefile (
    echo Cleaning previous build...
    nmake clean 2>nul || mingw32-make clean 2>nul
)

REM Generate Makefile
echo Generating Makefile...
qmake ..\ProMuxer.pro

if %errorlevel% neq 0 (
    echo ERROR: Failed to generate Makefile
    pause
    exit /b 1
)

REM Build the project
echo Building...
nmake 2>nul || mingw32-make

if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable location: build\release\ProMuxer.exe (or build\debug\ProMuxer.exe)
echo.
echo Note: Make sure FFmpeg is installed and available in PATH to use the application.
echo.
pause