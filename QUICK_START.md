# Pro Muxer - Quick Start Guide

## What is Pro Muxer?

Pro Muxer is a comprehensive Qt application for professional video stream muxing/remuxing. It features intelligent media analysis, batch processing, and advanced UI controls for handling both standard container files and raw bitstreams.

## Key Features

✅ **Intelligent Media Analysis** - Automatic detection of video/audio codecs, resolution, frame rate  
✅ **Raw Bitstream Support** - Manual codec specification for H.264/H.265 raw files  
✅ **Batch Processing** - Process multiple files with queue management  
✅ **Advanced UI** - Professional interface with detailed file information display  
✅ **Smart Logging** - Hierarchical logging system (Info/Warning/Error)  
✅ **Flexible Naming** - Custom prefix/suffix with conflict handling  
✅ **Container Compatibility** - Smart format recommendations  

## Prerequisites

1. **Qt 6.x** - Download from https://www.qt.io/download-qt-installer
2. **FFmpeg + FFprobe** - Download from https://ffmpeg.org/download.html
   - Extract FFmpeg and add the `bin` folder to your system PATH
   - Test with: `ffmpeg -version` and `ffprobe -version` in command prompt

## Quick Build & Run

### Method 1: Using the build script (Recommended)
1. Double-click `build.bat` to build the project
2. If successful, the executable will be in `build\release\ProMuxer.exe`

### Method 2: Manual build with Qt Creator
1. Open `ProMuxer.pro` in Qt Creator
2. Configure project with your Qt kit
3. Build and run

### Method 3: Command line build
```cmd
# Open Qt command prompt (from Start Menu -> Qt -> Qt 6.x.x -> Qt 6.x.x (MinGW...))
qmake ProMuxer.pro
mingw32-make  # or nmake if using MSVC
```

## How to Use

### 1. Add Files
- Click "Add Files" to select individual files
- Click "Add Folder" to add all videos from a folder  
- Or simply drag & drop files/folders into the file table

### 2. Analyze Media (Recommended)
- Click "Analyze Files" to automatically detect media information
- For raw bitstreams (.h264, .h265), use the dropdown to specify codec
- Review the detected information: resolution, frame rate, codecs, etc.

### 3. Configure Settings
- **Output Folder**: Choose where processed files will be saved
- **Output Format**: Select target container (MP4, MKV, MOV, WebM, TS)
- **Naming Rule**: Set custom prefix and suffix (default: "_muxed")
- **Conflict Handling**: Choose Auto Rename, Overwrite, or Skip existing files
- **Smart Compatibility**: Enable automatic format recommendations

### 4. Process Files
- Click "Start Processing" to begin batch muxing
- Monitor progress in the progress bar and status area
- Review detailed logs with different severity levels
- Use log filters to show/hide Info, Warning, and Error messages

## Advanced Features

### Raw Bitstream Handling
For raw H.264/H.265 files (.h264, .h265, .bin):
1. Files are automatically detected as raw streams
2. A dropdown menu appears in the "Video Codec" column
3. Select the correct codec type if auto-detection is wrong
4. The application will handle proper container packaging

### Log System
The application features a hierarchical logging system:
- **[INFO]** - General processing information (green/black)
- **[WARN]** - Non-critical warnings (orange) 
- **[ERROR]** - Critical errors requiring attention (red)

Use the checkboxes to filter log levels as needed.

### File Table Details
The main file table displays comprehensive information:
- File Name, Status, Video Codec, Resolution, Frame Rate
- Audio Codec, Duration, File Size, Output Name
- Real-time status updates during processing
- Editable output names (double-click)

## Supported Formats

**Input Formats**: MP4, MKV, AVI, MOV, WMV, FLV, WebM, M4V, 3GP, TS, H264, H265, BIN, 264, 265  
**Output Formats**: MP4, MKV, MOV, WebM, TS

**Video Codecs**: H.264, H.265, AV1, VP9, VP8, MPEG-2, ProRes, DNxHD, and more  
**Audio Codecs**: AAC, MP3, AC3, DTS, PCM, Vorbis, Opus, and more  

## Troubleshooting

### "FFmpeg/FFprobe executable not found"
- Ensure both FFmpeg and FFprobe are in your system PATH
- Test with `ffmpeg -version` and `ffprobe -version` in command prompt
- Try placing ffmpeg.exe and ffprobe.exe in the same folder as ProMuxer.exe

### "Analysis failed for file X"
- The file might be corrupted or in an unsupported format
- For raw streams, manually specify the codec using the dropdown
- Check the error details in the log window

### Build errors
- Make sure Qt 6.x is installed with all required modules (Core, Widgets)
- Ensure qmake is in PATH (usually in Qt\6.x.x\mingw_64\bin)
- For missing UI file errors, make sure MainWindow.ui is in the correct location

### Files won't process
- Verify input files are valid video/audio files
- Check if output folder is writable
- Look at the log output for specific error messages
- Ensure sufficient disk space for output files

## Project Structure

```
ffmpeg-media packaging/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── ui/
│   │   ├── MainWindow.h      # Main window header
│   │   ├── MainWindow.cpp    # Main window implementation  
│   │   └── MainWindow.ui     # Qt Designer UI file
│   └── core/
│       ├── FileProcessor.*   # Batch processing engine
│       ├── MuxingTask.*      # Individual file processing
│       └── MediaAnalyzer.*   # FFprobe integration
├── ProMuxer.pro              # Qt project file
├── CMakeLists.txt            # CMake build file (alternative)
├── build.bat                 # Windows build script
├── README.md                 # Detailed documentation
└── QUICK_START.md            # This file
```

This enhanced version provides a complete professional-grade muxing solution with all the advanced features requested in the original specification.