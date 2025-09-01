#include "MediaAnalyzer.h"
#include "../ui/MainWindow.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QStandardPaths>

MediaAnalyzer::MediaAnalyzer(QObject *parent)
    : QObject(parent)
    , m_probeProcess(nullptr)
    , m_analyzing(false)
{
    m_ffprobePath = findFFprobeExecutable();
}

MediaAnalyzer::~MediaAnalyzer()
{
    stop();
}

void MediaAnalyzer::analyzeFile(int index, const QString &filePath)
{
    AnalysisTask task;
    task.index = index;
    task.filePath = filePath;
    m_taskQueue.enqueue(task);
    
    if (!m_analyzing) {
        processNextFile();
    }
}

void MediaAnalyzer::analyzeFiles(const QStringList &files)
{
    for (int i = 0; i < files.size(); ++i) {
        analyzeFile(i, files[i]);
    }
}

void MediaAnalyzer::stop()
{
    if (m_probeProcess && m_probeProcess->state() == QProcess::Running) {
        m_probeProcess->kill();
        m_probeProcess->waitForFinished(1000);
    }
    
    m_taskQueue.clear();
    m_analyzing = false;
}

void MediaAnalyzer::processNextFile()
{
    if (m_taskQueue.isEmpty()) {
        m_analyzing = false;
        emit allAnalysisFinished();
        return;
    }
    
    m_analyzing = true;
    m_currentTask = m_taskQueue.dequeue();
    
    // Check if it's a raw stream file
    if (isRawStreamFile(m_currentTask.filePath)) {
        // For raw streams, we can't analyze much, provide default info
        MediaInfo info = createDefaultMediaInfo(m_currentTask.filePath);
        info.isRawStream = true;
        emit analysisFinished(m_currentTask.index, info);
        processNextFile();
        return;
    }
    
    if (m_ffprobePath.isEmpty()) {
        emit analysisError(m_currentTask.index, "FFprobe not found");
        processNextFile();
        return;
    }
    
    if (!m_probeProcess) {
        m_probeProcess = new QProcess(this);
        connect(m_probeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &MediaAnalyzer::onProbeFinished);
        connect(m_probeProcess, &QProcess::errorOccurred, this, &MediaAnalyzer::onProbeError);
    }
    
    QStringList arguments;
    arguments << "-v" << "quiet"
              << "-print_format" << "json"
              << "-show_format"
              << "-show_streams"
              << QDir::toNativeSeparators(m_currentTask.filePath);
    
    m_probeProcess->start(m_ffprobePath, arguments);
}

void MediaAnalyzer::onProbeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        QString output = QString::fromUtf8(m_probeProcess->readAllStandardOutput());
        MediaInfo info = parseFFprobeOutput(output);
        emit analysisFinished(m_currentTask.index, info);
    } else {
        QString error = QString::fromUtf8(m_probeProcess->readAllStandardError());
        emit analysisError(m_currentTask.index, QString("FFprobe failed: %1").arg(error));
    }
    
    processNextFile();
}

void MediaAnalyzer::onProbeError(QProcess::ProcessError error)
{
    QString errorString;
    switch (error) {
    case QProcess::FailedToStart:
        errorString = "Failed to start FFprobe";
        break;
    case QProcess::Crashed:
        errorString = "FFprobe crashed";
        break;
    default:
        errorString = "FFprobe process error";
        break;
    }
    
    emit analysisError(m_currentTask.index, errorString);
    processNextFile();
}

MediaInfo MediaAnalyzer::parseFFprobeOutput(const QString &output)
{
    MediaInfo info;
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return info;
    }
    
    QJsonObject root = doc.object();
    
    // Parse format information
    if (root.contains("format")) {
        QJsonObject format = root["format"].toObject();
        
        if (format.contains("duration")) {
            double duration = format["duration"].toString().toDouble();
            info.duration = formatDuration(duration);
        }
        
        if (format.contains("size")) {
            qint64 size = format["size"].toString().toLongLong();
            info.fileSize = formatFileSize(size);
        }
        
        if (format.contains("bit_rate")) {
            int bitrate = format["bit_rate"].toString().toInt();
            info.bitrate = QString::number(bitrate / 1000) + " kbps";
        }
    }
    
    // Parse streams information
    if (root.contains("streams")) {
        QJsonArray streams = root["streams"].toArray();
        
        for (const QJsonValue &streamValue : streams) {
            QJsonObject stream = streamValue.toObject();
            QString codecType = stream["codec_type"].toString();
            
            if (codecType == "video" && info.videoCodec.isEmpty()) {
                info.videoCodec = stream["codec_name"].toString().toUpper();
                
                if (stream.contains("width") && stream.contains("height")) {
                    int width = stream["width"].toInt();
                    int height = stream["height"].toInt();
                    info.resolution = QString("%1x%2").arg(width).arg(height);
                }
                
                if (stream.contains("r_frame_rate")) {
                    QString frameRate = stream["r_frame_rate"].toString();
                    if (frameRate.contains("/")) {
                        QStringList parts = frameRate.split("/");
                        if (parts.size() == 2) {
                            double fps = parts[0].toDouble() / parts[1].toDouble();
                            info.frameRate = QString::number(fps, 'f', 2) + " fps";
                        }
                    }
                }
                
                // Parse bit depth
                if (stream.contains("bits_per_raw_sample")) {
                    int bitDepth = stream["bits_per_raw_sample"].toInt();
                    info.bitDepth = QString::number(bitDepth) + " bit";
                } else if (stream.contains("pix_fmt")) {
                    QString pixFmt = stream["pix_fmt"].toString();
                    // Common pixel format to bit depth mapping
                    if (pixFmt.contains("yuv420p10") || pixFmt.contains("p010")) {
                        info.bitDepth = "10 bit";
                    } else if (pixFmt.contains("yuv420p12")) {
                        info.bitDepth = "12 bit";
                    } else if (pixFmt.contains("yuv420p16")) {
                        info.bitDepth = "16 bit";
                    } else if (pixFmt.contains("yuv420p")) {
                        info.bitDepth = "8 bit";
                    } else {
                        info.bitDepth = "Unknown";
                    }
                }
                
                // Parse color space
                if (stream.contains("color_space")) {
                    QString colorSpace = stream["color_space"].toString();
                    if (colorSpace == "bt709") {
                        info.colorSpace = "Rec. 709";
                    } else if (colorSpace == "bt2020nc" || colorSpace == "bt2020c") {
                        info.colorSpace = "Rec. 2020";
                    } else if (colorSpace == "smpte170m") {
                        info.colorSpace = "SMPTE 170M";
                    } else if (colorSpace == "bt470bg") {
                        info.colorSpace = "PAL";
                    } else if (!colorSpace.isEmpty()) {
                        info.colorSpace = colorSpace.toUpper();
                    } else {
                        info.colorSpace = "Unknown";
                    }
                } else {
                    // Try to infer from resolution for common cases
                    if (stream.contains("width")) {
                        int width = stream["width"].toInt();
                        if (width >= 1920) {
                            info.colorSpace = "Rec. 709"; // HD/UHD default
                        } else {
                            info.colorSpace = "Rec. 601"; // SD default
                        }
                    } else {
                        info.colorSpace = "Unknown";
                    }
                }
            }
            else if (codecType == "audio" && info.audioCodec.isEmpty()) {
                info.audioCodec = stream["codec_name"].toString().toUpper();
                
                if (stream.contains("channels")) {
                    int channels = stream["channels"].toInt();
                    info.audioCodec += QString(" (%1ch)").arg(channels);
                }
                
                if (stream.contains("sample_rate")) {
                    int sampleRate = stream["sample_rate"].toInt();
                    info.audioCodec += QString(" %1Hz").arg(sampleRate);
                }
            }
        }
    }
    
    info.analyzed = true;
    return info;
}

QString MediaAnalyzer::formatDuration(double seconds)
{
    int hours = static_cast<int>(seconds) / 3600;
    int minutes = (static_cast<int>(seconds) % 3600) / 60;
    int secs = static_cast<int>(seconds) % 60;
    
    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes).arg(secs, 2, 10, QChar('0'));
    }
}

QString MediaAnalyzer::formatFileSize(qint64 size)
{
    if (size < 1024) {
        return QString::number(size) + " B";
    } else if (size < 1024 * 1024) {
        return QString::number(size / 1024.0, 'f', 1) + " KB";
    } else if (size < 1024 * 1024 * 1024) {
        return QString::number(size / (1024.0 * 1024.0), 'f', 1) + " MB";
    } else {
        return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }
}

QString MediaAnalyzer::findFFprobeExecutable()
{
    QProcess process;
    
#ifdef Q_OS_WIN
    QString program = "ffprobe.exe";
#else
    QString program = "ffprobe";
#endif
    
    // Check if ffprobe is available in PATH by running -version
    process.start(program, QStringList() << "-version");
    if (process.waitForStarted(3000) && process.waitForFinished(3000)) {
        if (process.exitCode() == 0) {
            // Parse version output to extract version and year (similar to ffmpeg)
            QString output = QString::fromUtf8(process.readAllStandardOutput());
            parseAndLogFFprobeVersion(output);
            return program;
        }
    }
    
    return QString(); // Not found in PATH
}

QString MediaAnalyzer::getResolutionDescription(int width, int height)
{
    if (width == 7680 && height == 4320) return "(8K UHD)";
    if (width == 3840 && height == 2160) return "(4K UHD)";
    if (width == 2560 && height == 1440) return "(QHD)";
    if (width == 1920 && height == 1080) return "(FHD)";
    if (width == 1280 && height == 720) return "(HD)";
    if (width == 854 && height == 480) return "(SD)";
    if (width == 640 && height == 480) return "(VGA)";
    
    // Check for common aspect ratios
    double ratio = (double)width / height;
    if (abs(ratio - 16.0/9.0) < 0.01) return "(16:9)";
    if (abs(ratio - 4.0/3.0) < 0.01) return "(4:3)";
    if (abs(ratio - 21.0/9.0) < 0.01) return "(21:9)";
    
    return ""; // No specific description
}

bool MediaAnalyzer::isRawStreamFile(const QString &filePath)
{
    QStringList rawExtensions;
    rawExtensions << "h264" << "h265" << "hevc" << "bin" << "264" << "265";
    
    QString extension = QFileInfo(filePath).suffix().toLower();
    return rawExtensions.contains(extension);
}

MediaInfo MediaAnalyzer::createDefaultMediaInfo(const QString &filePath)
{
    MediaInfo info;
    QFileInfo fileInfo(filePath);
    
    info.fileSize = formatFileSize(fileInfo.size());
    info.duration = "Unknown";
    info.audioCodec = "None";
    
    // Enhanced intelligent parsing from filename patterns
    QString fileName = fileInfo.fileName().toLower();
    QString baseName = fileInfo.baseName().toLower();
    QString extension = fileInfo.suffix().toLower();
    
    // Parse video codec from extension and filename
    if (extension == "h264" || extension == "264" || fileName.contains("avc") || fileName.contains("h264")) {
        info.videoCodec = "H.264";
    } else if (extension == "h265" || extension == "hevc" || extension == "265" || 
               fileName.contains("hevc") || fileName.contains("h265")) {
        info.videoCodec = "H.265";
    } else if (fileName.contains("av1")) {
        info.videoCodec = "AV1";
    } else if (fileName.contains("vp9")) {
        info.videoCodec = "VP9";
    } else {
        info.videoCodec = "Unknown";
    }
    
    // Parse bit depth with priority: explicit patterns > codec defaults
    if (fileName.contains("10bit") || fileName.contains("10-bit") || fileName.contains("_10bit")) {
        info.bitDepth = "10 bit";
    } else if (fileName.contains("12bit") || fileName.contains("12-bit")) {
        info.bitDepth = "12 bit";
    } else if (fileName.contains("16bit") || fileName.contains("16-bit")) {
        info.bitDepth = "16 bit";
    } else if (fileName.contains("8bit") || fileName.contains("8-bit")) {
        info.bitDepth = "8 bit";
    } else {
        // Default based on codec
        if (info.videoCodec == "H.265" || info.videoCodec == "AV1") {
            info.bitDepth = "10 bit"; // Modern codecs often 10-bit
        } else {
            info.bitDepth = "8 bit"; // Legacy default
        }
    }
    
    // Parse resolution with multiple pattern matching strategies
    QRegularExpression resolutionRegex("(\\d{3,4})x(\\d{3,4})");
    QRegularExpressionMatch resMatch = resolutionRegex.match(fileName);
    if (resMatch.hasMatch()) {
        int width = resMatch.captured(1).toInt();
        int height = resMatch.captured(2).toInt();
        QString resDescription = getResolutionDescription(width, height);
        info.resolution = QString("%1x%2 %3").arg(width).arg(height).arg(resDescription);
    } else {
        // Try common resolution keywords
        if (fileName.contains("8k") || fileName.contains("4320")) {
            info.resolution = "7680x4320 (8K UHD)";
        } else if (fileName.contains("4k") || fileName.contains("2160") || fileName.contains("uhd")) {
            info.resolution = "3840x2160 (4K UHD)";
        } else if (fileName.contains("fhd") || fileName.contains("1080")) {
            info.resolution = "1920x1080 (FHD)";
        } else if (fileName.contains("hd") || fileName.contains("720")) {
            info.resolution = "1280x720 (HD)";
        } else if (fileName.contains("qhd") || fileName.contains("1440")) {
            info.resolution = "2560x1440 (QHD)";
        } else {
            info.resolution = "Unknown";
        }
    }
    
    // Parse frame rate with comprehensive pattern matching
    QRegularExpression fpsRegex("(\\d{2,3}(?:\\.\\d+)?)fps");
    QRegularExpressionMatch fpsMatch = fpsRegex.match(fileName);
    if (fpsMatch.hasMatch()) {
        double fps = fpsMatch.captured(1).toDouble();
        info.frameRate = QString("%1 fps").arg(fps, 0, 'f', fps == (int)fps ? 0 : 3);
    } else {
        // Try specific frame rate patterns
        if (fileName.contains("23.976") || fileName.contains("23976")) {
            info.frameRate = "23.976 fps";
        } else if (fileName.contains("29.97") || fileName.contains("2997")) {
            info.frameRate = "29.970 fps";
        } else if (fileName.contains("59.94") || fileName.contains("5994")) {
            info.frameRate = "59.940 fps";
        } else if (fileName.contains("120p") || fileName.contains("120fps")) {
            info.frameRate = "120.000 fps";
        } else if (fileName.contains("60p") || fileName.contains("60fps")) {
            info.frameRate = "60.000 fps";
        } else if (fileName.contains("50p") || fileName.contains("50fps")) {
            info.frameRate = "50.000 fps";
        } else if (fileName.contains("30p") || fileName.contains("30fps")) {
            info.frameRate = "30.000 fps";
        } else if (fileName.contains("25p") || fileName.contains("25fps")) {
            info.frameRate = "25.000 fps";
        } else if (fileName.contains("24p") || fileName.contains("24fps")) {
            info.frameRate = "24.000 fps";
        } else {
            info.frameRate = "Unknown";
        }
    }
    
    // Parse color space based on resolution and keywords
    if (fileName.contains("hdr") || fileName.contains("rec2020") || fileName.contains("bt2020")) {
        info.colorSpace = "Rec. 2020 (HDR)";
    } else if (fileName.contains("p3") || fileName.contains("dci-p3")) {
        info.colorSpace = "DCI-P3";
    } else if (fileName.contains("rec601") || fileName.contains("bt601")) {
        info.colorSpace = "Rec. 601 (SDTV)";
    } else {
        // Infer from resolution
        if (info.resolution.contains("4K") || info.resolution.contains("8K") || info.resolution.contains("2160")) {
            info.colorSpace = "Rec. 709 (sRGB)"; // UHD default
        } else if (info.resolution.contains("1080") || info.resolution.contains("720")) {
            info.colorSpace = "Rec. 709 (sRGB)"; // HD default
        } else if (info.resolution.contains("480")) {
            info.colorSpace = "Rec. 601 (SDTV)"; // SD default
        } else {
            info.colorSpace = "Rec. 709 (sRGB)"; // Safe default
        }
    }
    
    return info;
}

void MediaAnalyzer::parseAndLogFFprobeVersion(const QString &versionOutput)
{
    // Parse FFprobe version information from --version output
    // Example output: "ffprobe version 4.4.2 Copyright (c) 2000-2021 the FFmpeg developers"
    
    QStringList lines = versionOutput.split('\n');
    if (lines.isEmpty()) {
        // No need to emit signal here as MediaAnalyzer doesn't have logMessage signal
        qDebug() << "FFprobe found in PATH, but version information unavailable";
        return;
    }
    
    QString firstLine = lines.first();
    
    // Extract version number
    QRegularExpression versionRegex(R"(ffprobe version ([\d\.\w-]+))");
    QRegularExpressionMatch versionMatch = versionRegex.match(firstLine);
    QString version = "Unknown";
    if (versionMatch.hasMatch()) {
        version = versionMatch.captured(1);
    }
    
    // Extract copyright year range
    QRegularExpression yearRegex(R"(Copyright \(c\) (\d{4})-(\d{4}))");
    QRegularExpressionMatch yearMatch = yearRegex.match(firstLine);
    QString yearRange = "Unknown";
    if (yearMatch.hasMatch()) {
        QString startYear = yearMatch.captured(1);
        QString endYear = yearMatch.captured(2);
        yearRange = QString("%1-%2").arg(startYear).arg(endYear);
    }
    
    qDebug() << QString("FFprobe found in PATH - Version: %1, Release Years: %2")
                .arg(version).arg(yearRange);
}
