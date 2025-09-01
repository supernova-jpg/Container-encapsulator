@echo off
echo Testing FFmpeg with raw H.265 file...
echo.

REM Test basic FFmpeg functionality
ffmpeg.exe -version >nul 2>&1
if errorlevel 1 (
    echo ERROR: FFmpeg not found in PATH
    exit /b 1
)

REM Create a test command similar to what ProMuxer would generate
set INPUT=test_h265_1920x1080_30fps_420p10le.265
set OUTPUT=test_output.mp4

echo Test 1: Basic raw H.265 to MP4 conversion
ffmpeg.exe -f hevc -framerate 30 -s 1920x1080 -i "%INPUT%" -c:v copy -f mp4 -movflags +faststart "%OUTPUT%" -y

echo.
echo Test 2: With thread limit and additional parameters
ffmpeg.exe -f hevc -framerate 30 -s 1920x1080 -threads 4 -probesize 50M -analyzeduration 50M -fflags +genpts -i "%INPUT%" -c:v copy -c:a copy -c:s copy -f mp4 -movflags +faststart "%OUTPUT%_v2" -y

echo.
echo Test 3: Minimal command
ffmpeg.exe -f hevc -i "%INPUT%" -c copy "%OUTPUT%_v3" -y

echo.
echo Tests completed. Check for any errors above.
pause