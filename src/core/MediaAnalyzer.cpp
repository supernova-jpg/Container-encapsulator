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
    QStringList possiblePaths;
    
#ifdef Q_OS_WIN
    possiblePaths << "ffprobe.exe"
                  << "C:/ffmpeg/bin/ffprobe.exe"
                  << "C:/Program Files/ffmpeg/bin/ffprobe.exe"
                  << "C:/Program Files (x86)/ffmpeg/bin/ffprobe.exe";
#else
    possiblePaths << "ffprobe"
                  << "/usr/bin/ffprobe"
                  << "/usr/local/bin/ffprobe"
                  << "/opt/homebrew/bin/ffprobe";
#endif
    
    // Try to find in PATH first
    QProcess process;
    QString program = possiblePaths.first();
    
    process.start(program, QStringList() << "-version");
    if (process.waitForStarted(3000) && process.waitForFinished(3000)) {
        if (process.exitCode() == 0) {
            return program;
        }
    }
    
    // Try specific paths
    for (int i = 1; i < possiblePaths.size(); ++i) {
        const QString &path = possiblePaths[i];
        if (QFile::exists(path)) {
            process.start(path, QStringList() << "-version");
            if (process.waitForStarted(3000) && process.waitForFinished(3000)) {
                if (process.exitCode() == 0) {
                    return path;
                }
            }
        }
    }
    
    return QString(); // Not found
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
    info.resolution = "Unknown";
    info.frameRate = "Unknown";
    info.audioCodec = "None";
    
    // Try to guess codec from extension
    QString extension = fileInfo.suffix().toLower();
    if (extension == "h264" || extension == "264") {
        info.videoCodec = "H.264";
    } else if (extension == "h265" || extension == "hevc" || extension == "265") {
        info.videoCodec = "H.265";
    } else {
        info.videoCodec = "Unknown";
    }
    
    return info;
}