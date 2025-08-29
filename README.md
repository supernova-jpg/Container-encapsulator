# Pro Muxer - Professional Video Stream Muxing Tool

A simple and efficient Qt-based desktop application for batch video file muxing/remuxing using FFmpeg.

## Features

- **Batch Processing**: Process multiple video files at once
- **Drag & Drop Support**: Simply drag files or folders into the application
- **Multiple Output Formats**: Support for MP4, MKV, MOV, WebM, and TS
- **Stream Copy**: Fast processing using stream copy (no re-encoding)
- **Progress Monitoring**: Real-time progress tracking and detailed logs
- **Smart File Handling**: Automatic file renaming to avoid conflicts
- **Modern UI**: Dark theme with intuitive interface

## Supported Input Formats

- Standard container files: MP4, MKV, AVI, MOV, WMV, FLV, WebM, M4V, 3GP, TS
- Raw bitstream files: H.264, H.265, BIN files

## Requirements

### System Requirements
- Windows 10 or later
- Qt 6.x framework
- FFmpeg executable (must be available in PATH or installed)

### Development Requirements
- Qt 6.x (Core, Widgets modules)
- CMake 3.16 or later
- C++17 compatible compiler (MSVC 2019+, GCC 8+, Clang 8+)

## Installation

### Option 1: Download Pre-built Binary
1. Download the latest release from the releases page
2. Extract to desired location
3. Ensure FFmpeg is installed and available in system PATH
4. Run `ProMuxer.exe`

### Option 2: Build from Source

#### Prerequisites
```bash
# Install Qt 6 (Windows - using Qt installer)
# Download from: https://www.qt.io/download-qt-installer

# Install FFmpeg (Windows)
# Download from: https://ffmpeg.org/download.html
# Add FFmpeg to system PATH
```

#### Build Steps
```bash
# Clone or download the source code
cd "ffmpeg-media packaging"

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" ..

# Build
cmake --build . --config Release

# The executable will be in Release/ directory
```

## Usage

### Basic Workflow
1. **Launch the application**
2. **Add files** using one of these methods:
   - Click "Add Files" button to select individual files
   - Click "Add Folder" button to add all supported files from a folder
   - Drag and drop files or folders directly into the file list
3. **Configure settings**:
   - Set output folder (default: Documents/ProMuxer_Output)
   - Choose output format (MP4, MKV, MOV, WebM, TS)
   - Enable "Overwrite existing files" if needed
4. **Start processing** by clicking "Start Processing"
5. **Monitor progress** in the progress bar and log window

### Output Files
- Output files are named: `[original_name]_muxed.[format]`
- If a file already exists and overwrite is disabled, files are auto-renamed with a counter
- All original metadata and streams are preserved during muxing

### FFmpeg Integration
The application uses FFmpeg command-line with the following approach:
- Stream copy mode (`-c copy`) for fast processing without re-encoding
- Maps all streams (`-map 0`) to preserve audio, video, and subtitle tracks
- Format-specific optimizations (e.g., `faststart` for MP4)

## Troubleshooting

### Common Issues

**"FFmpeg executable not found"**
- Ensure FFmpeg is installed and added to system PATH
- Try placing `ffmpeg.exe` in the same folder as the application
- Check FFmpeg installation with: `ffmpeg -version`

**Files not processing**
- Verify input files are valid video files
- Check log output for specific error messages
- Ensure sufficient disk space for output files

**Application won't start**
- Verify Qt runtime libraries are available
- Check that Visual C++ Redistributable is installed (Windows)

### Supported File Extensions
Input: `.mp4`, `.mkv`, `.avi`, `.mov`, `.wmv`, `.flv`, `.webm`, `.m4v`, `.3gp`, `.ts`, `.h264`, `.h265`, `.bin`

Output: `.mp4`, `.mkv`, `.mov`, `.webm`, `.ts`

## Technical Details

### Architecture
- **Qt Framework**: Modern cross-platform UI framework
- **MVC Pattern**: Separation of UI, business logic, and data
- **Multi-threading**: Background processing without UI blocking
- **Stream Processing**: Efficient queue-based file processing

### Key Components
- `MainWindow`: Primary user interface
- `FileProcessor`: Manages batch processing queue
- `MuxingTask`: Handles individual file processing with FFmpeg
- `CMakeLists.txt`: Cross-platform build configuration

## License

This project is open source. Please check the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

## Version History

### v1.0.0
- Initial release
- Batch file processing
- Multiple output format support
- Drag & drop interface
- Progress monitoring and logging