# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Pro Muxer is a professional Qt C++ desktop application for batch video stream muxing/remuxing. It's designed for Windows 10+ with cross-platform capability, utilizing Qt 6.x framework and FFmpeg for video processing.

## Build Commands

**Quick Build (Recommended)**:
```bash
build.bat
# Output: build/release/ProMuxer.exe
```

**Command Line Build**:
```bash
# qmake approach
qmake ProMuxer.pro
mingw32-make    # MinGW
nmake          # MSVC

# CMake approach
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" ..
cmake --build . --config Release
```

**Run Tests**:
```bash
cd tests
qmake ProMuxerTests.pro
mingw32-make
# Run: ProMuxerTests.exe
```

## Architecture

**Structure**: Model-View-Controller (MVC) with Qt Widgets
- `src/ui/MainWindow.*` - Primary UI with dark theme, drag-drop, real-time progress
- `src/core/FileProcessor.*` - Queue-based batch processing engine
- `src/core/MuxingTask.*` - Individual FFmpeg task execution
- `src/core/MediaAnalyzer.*` - FFprobe integration with smart fallback system

**Key Data Flow**:
1. MediaAnalyzer uses FFprobe for metadata extraction with intelligent fallback
2. FileProcessor manages batch queue and FFmpeg command generation
3. MuxingTask handles individual file processing with progress parsing
4. MainWindow coordinates UI updates and user interactions

## Technology Stack

- **Framework**: Qt 6.x (Core, Widgets)
- **Language**: C++17
- **Build Systems**: qmake (primary), CMake (alternative)
- **Dependencies**: FFmpeg/FFprobe (runtime)
- **Target**: Windows 10+ (cross-platform capable)

## Development Patterns

**Code Standards**:
- C++17 compliance required
- Qt naming conventions (m_ prefix for members)
- Signal-slot architecture for component communication
- RAII resource management

**UI Design**:
- Dark theme with Fusion style
- Segoe UI typography at 10-11pt
- Professional color scheme (dark grays, orange accents)
- Theme-aware logging with hierarchical levels (Info/Warning/Error)

**Error Handling**:
- Smart fallback system for media analysis failures
- Graceful FFmpeg/FFprobe detection
- Industry-grade filename pattern recognition for metadata
- Comprehensive fallback mechanisms for raw bitstream files

## File Support

**Input Formats**: MP4, MKV, AVI, MOV, WMV, FLV, WebM, M4V, 3GP, TS, H264, H265, BIN, 264, 265
**Output Formats**: MP4, MKV, MOV, WebM, TS

## Special Considerations

**Advanced Features**:
- Intelligent media analysis with automatic detection + smart fallback
- Raw bitstream handling with manual codec specification
- Batch processing with queue management and progress tracking
- Professional metadata handling with comprehensive preset systems

**Runtime Dependencies**:
- FFmpeg and FFprobe executables (PATH or same directory)
- Qt 6.x runtime libraries
- Visual C++ Redistributable (Windows)

**Testing**: Qt TestLib-based suite covering FFmpeg command generation, media analysis, and UI workflow

**Deployment**: Windows-first with cross-platform capability, includes resource files for professional deployment