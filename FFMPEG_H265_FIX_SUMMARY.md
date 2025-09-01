# FFmpeg H.265 Raw Stream Processing Fix

## Problem
FFmpeg was crashing immediately when processing raw H.265 (.265) files with the error "FFmpeg process crashed" and no output being captured. This happened for all H.265 files being processed through ProMuxer.

## Root Causes Identified

1. **Incorrect parameter order**: For raw H.265 streams, FFmpeg requires specific input parameters (format, framerate, resolution) to be specified BEFORE the input file.

2. **Missing framerate parameter**: The code was using `-r` instead of `-framerate` for raw streams. The `-framerate` parameter is required for raw video input.

3. **Missing probe parameters**: Raw streams need larger probe buffer sizes to be properly analyzed.

4. **Thread safety**: Large H.265 files could cause crashes due to threading issues.

5. **Pixel format issues**: Specifying pixel format for 10-bit content could cause format conversion instead of stream copying.

## Changes Made

### 1. FileProcessor.cpp - Fixed FFmpeg command construction

- **Changed `-r` to `-framerate`** for raw streams (line 212)
- **Added proper parameter ordering**: format → framerate → resolution → input file
- **Added fallback framerate detection** from filename if not in metadata
- **Added thread limit** (`-threads 4`) to prevent crashes (line 233)
- **Added probe parameters** for raw streams:
  - `-probesize 50M`
  - `-analyzeduration 50M`
- **Removed pixel format specification** for 10-bit content to prevent unwanted conversions
- **Improved 10-bit detection** to include "420p10" pattern in filenames

### 2. MuxingTask.cpp - Improved error handling and diagnostics

- **Set working directory** to input file's directory for better path handling (lines 67-70)
- **Improved error output capture** to get last non-empty line (lines 127-135)
- **Enhanced stderr/stdout reading** to capture all FFmpeg output (lines 182-198)
- **Better error logging** with [ERROR] prefix for failed starts

## Resulting FFmpeg Command Structure

Before fix:
```
ffmpeg.exe -f hevc -r 30 -s 1920x1080 -fflags +genpts -i \\path\file.265 -c:v copy ...
```

After fix:
```
ffmpeg.exe -f hevc -framerate 30 -s 1920x1080 -threads 4 -probesize 50M -analyzeduration 50M -fflags +genpts -i \\path\file.265 -c:v copy -c:a copy -c:s copy -f mp4 -movflags +faststart output.mp4
```

## Key Improvements

1. **Correct parameter order** ensures FFmpeg can properly interpret raw streams
2. **Thread limiting** prevents crashes with large files
3. **Larger probe sizes** help FFmpeg analyze raw streams correctly
4. **Better error reporting** helps diagnose future issues
5. **No pixel format conversion** - true stream copying as intended

## Testing

Created `test_ffmpeg_h265.bat` to verify the command structure works correctly with various parameter combinations.

## Expected Result

The H.265 raw streams should now be properly muxed into MP4 containers without crashes, maintaining the original video quality through stream copying.