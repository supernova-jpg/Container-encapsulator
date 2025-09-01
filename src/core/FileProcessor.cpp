#include "FileProcessor.h"
#include "MuxingTask.h"
#include "../ui/MainWindow.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QProcess>

FileProcessor::FileProcessor(QObject *parent)
    : QObject(parent)
    , m_currentTask(nullptr)
    , m_overwrite(false)
    , m_processing(false)
    , m_currentIndex(0)
    , m_totalFiles(0)
{
    m_ffmpegPath = findFFmpegExecutable();
}

FileProcessor::~FileProcessor()
{
    stop();
}

void FileProcessor::processFiles(const QStringList &files, const QString &outputFolder, 
                               const QString &format, const QVector<MediaInfo> &mediaInfos,
                               bool overwrite)
{
    if (m_processing) {
        emit logMessage("Already processing files. Stop current operation first.");
        return;
    }
    
    m_files = files;
    m_outputFolder = outputFolder;
    m_outputFormat = format;
    m_mediaInfos = mediaInfos;
    m_overwrite = overwrite;
    m_processing = true;
    m_currentIndex = 0;
    m_totalFiles = files.size();
    
    // Clear any existing tasks
    qDeleteAll(m_taskQueue);
    m_taskQueue.clear();
    
    emit logMessage(QString("Starting to process %1 files...").arg(m_totalFiles));
    emit logMessage(QString("Output folder: %1").arg(m_outputFolder));
    emit logMessage(QString("Output format: %1").arg(m_outputFormat));
    emit logMessage(QString("FFmpeg path: %1").arg(m_ffmpegPath.isEmpty() ? "Not found" : m_ffmpegPath));
    
    if (m_ffmpegPath.isEmpty()) {
        emit logMessage("[ERROR] FFmpeg executable not found. Please ensure FFmpeg is installed and in PATH.");
        m_processing = false;
        return;
    }
    
    // Create tasks for all files
    for (int i = 0; i < m_files.size(); ++i) {
        const QString &inputFile = m_files[i];
        QFileInfo inputInfo(inputFile);
        QString outputName = inputInfo.completeBaseName() + "_muxed." + m_outputFormat;
        QString outputFile = QDir(m_outputFolder).absoluteFilePath(outputName);
        
        // Check if output file exists and handle accordingly
        if (QFile::exists(outputFile) && !m_overwrite) {
            // Auto-rename
            int counter = 1;
            QString baseName = inputInfo.completeBaseName() + "_muxed";
            do {
                outputName = QString("%1_%2.%3").arg(baseName).arg(counter).arg(m_outputFormat);
                outputFile = QDir(m_outputFolder).absoluteFilePath(outputName);
                counter++;
            } while (QFile::exists(outputFile));
        }
        
        MuxingTask *task = new MuxingTask(this);
        task->setFiles(inputFile, outputFile);

        MediaInfo mediaInfo = (i < m_mediaInfos.size()) ? m_mediaInfos[i] : MediaInfo();
        QStringList commandArgs = buildFFmpegCommand(inputFile, outputFile, m_outputFormat, mediaInfo);
        task->setCommandAndArgs(m_ffmpegPath, commandArgs);
        
        connect(task, &MuxingTask::finished, this, &FileProcessor::onTaskFinished);
        connect(task, &MuxingTask::logMessage, this, &FileProcessor::logMessage);
        
        m_taskQueue.enqueue(task);
    }
    
    // Start processing
    processNextFile();
}

void FileProcessor::stop()
{
    if (!m_processing) {
        return;
    }
    
    emit logMessage("Stopping processing...");
    
    if (m_currentTask) {
        m_currentTask->stop();
        m_currentTask = nullptr;
    }
    
    qDeleteAll(m_taskQueue);
    m_taskQueue.clear();
    
    m_processing = false;
    emit finished();
}

void FileProcessor::processNextFile()
{
    if (m_taskQueue.isEmpty()) {
        m_processing = false;
        emit logMessage("Batch processing completed. Review individual file results above.");
        emit finished();
        return;
    }
    
    m_currentTask = m_taskQueue.dequeue();
    
    emit logMessage(QString("Processing file %1/%2: %3")
                   .arg(m_currentIndex + 1)
                   .arg(m_totalFiles)
                   .arg(QFileInfo(m_currentTask->getInputFile()).fileName()));
    
    emit progress(m_currentIndex, m_totalFiles, m_currentTask->getInputFile());
    
    m_currentTask->start();
}

void FileProcessor::onTaskFinished(bool success, const QString &message)
{
    if (!m_currentTask) {
        return;
    }
    
    QString inputFile = m_currentTask->getInputFile();
    QString outputFile = m_currentTask->getOutputFile();
    
    if (success) {
        emit logMessage(QString("✓ Successfully processed: %1 -> %2")
                       .arg(QFileInfo(inputFile).fileName())
                       .arg(QFileInfo(outputFile).fileName()));
    } else {
        // This is a critical error that should be logged as ERROR level
        emit logMessage(QString("[ERROR] ✗ Failed to process: %1 - %2")
                       .arg(QFileInfo(inputFile).fileName())
                       .arg(message));
    }
    
    m_currentTask->deleteLater();
    m_currentTask = nullptr;
    m_currentIndex++;
    
    if (m_processing) {
        processNextFile();
    }
}

QStringList FileProcessor::buildFFmpegCommand(const QString &inputFile, const QString &outputFile,
                                              const QString &format, const MediaInfo &mediaInfo)
{
    QStringList args;

    args << "-fflags" << "+genpts";

    // Determine if this is a raw elementary stream (e.g., .h264/.h265/.bin)
    QString extension = QFileInfo(inputFile).suffix().toLower();
    bool looksRawByExt = QStringList({"h264", "h265", "hevc", "bin", "264", "265"}).contains(extension);
    bool isRaw = mediaInfo.isRawStream || looksRawByExt;

    // Heuristically determine codec for raw streams
    auto determineIsHevc = [&]() -> bool {
        QString codec = mediaInfo.videoCodec.toLower();
        QString name = QFileInfo(inputFile).fileName().toLower();
        if (codec.contains("265") || codec.contains("hevc")) return true;
        if (codec.contains("264") || codec.contains("h264") || codec.contains("avc")) return false;
        if (name.contains("hevc") || name.contains("h265") || extension == "265" || extension == "hevc") return true;
        if (name.contains("h264") || name.contains("avc") || extension == "264" || extension == "h264") return false;
        // Default to HEVC for unknown .bin if filename hints at it, otherwise assume H.264
        return name.contains("hevc") || name.contains("265");
    };

    if (isRaw) {
        // Set input frame rate for raw streams using -r (input option)
        if (!mediaInfo.frameRate.isEmpty() && !mediaInfo.frameRate.contains("Unknown", Qt::CaseInsensitive)) {
            QString frameRateText = mediaInfo.frameRate;
            frameRateText.remove(" fps").remove("fps");
            bool ok = false;
            double fps = frameRateText.toDouble(&ok);
            if (ok && fps > 0.0) {
                args << "-r" << QString::number(fps);
            }
        }

        // Explicitly set demuxer for raw elementary streams
        bool hevc = determineIsHevc();
        args << "-f" << (hevc ? "hevc" : "h264");
    }

    // Input file must come after any input options (-r, -f, etc.)
    args << "-i" << QDir::toNativeSeparators(inputFile);

    // Stream copy only (no re-encoding)
    args << "-c:v" << "copy";

    // Preserve color metadata when provided
    QString cs = mediaInfo.colorSpace.toLower();
    bool hasColorInfo = !mediaInfo.colorSpace.isEmpty() && !cs.contains("unknown", Qt::CaseInsensitive);
    if (hasColorInfo) {
        // Container-level color tags (visible to many players)
        if (cs.contains("2020")) {
            args << "-color_primaries" << "bt2020"
                 << "-colorspace" << "bt2020nc"
                 << "-color_trc" << "smpte2084";
        } else if (cs.contains("709")) {
            args << "-color_primaries" << "bt709"
                 << "-colorspace" << "bt709"
                 << "-color_trc" << "bt709";
        } else if (cs.contains("601") || cs.contains("smpte 170m") || cs.contains("smpte")) {
            args << "-color_primaries" << "smpte170m"
                 << "-colorspace" << "smpte170m"
                 << "-color_trc" << "smpte170m";
        }

        // Bitstream-level color metadata injection via bsf (retains c:v copy)
        QString bsf;
        if (isRaw) {
            bool hevc = determineIsHevc();
            QString primaries, trc, matrix;
            if (cs.contains("2020")) {
                primaries = "bt2020"; trc = "smpte2084"; matrix = "bt2020nc";
            } else if (cs.contains("709")) {
                primaries = "bt709"; trc = "bt709"; matrix = "bt709";
            } else {
                primaries = "smpte170m"; trc = "smpte170m"; matrix = "smpte170m";
            }
            if (hevc) {
                bsf = QString("hevc_metadata=color_primaries=%1:transfer_characteristics=%2:matrix_coefficients=%3")
                          .arg(primaries, trc, matrix);
            } else {
                bsf = QString("h264_metadata=color_primaries=%1:transfer_characteristics=%2:matrix_coefficients=%3")
                          .arg(primaries, trc, matrix);
            }
        }
        if (!bsf.isEmpty()) {
            args << "-bsf:v" << bsf;
        }
    }

    if (m_overwrite) {
        args << "-y";
    }

    if (format.toLower() == "mp4") {
        args << "-f" << "mp4";
        args << "-movflags" << "faststart";

        // Improve compatibility for H.264/HEVC in MP4
        if (isRaw) {
            bool hevc = determineIsHevc();
            if (hevc) {
                args << "-tag:v" << "hvc1";
            } else {
                args << "-tag:v" << "avc1";
            }
        }
    } else if (format.toLower() == "mkv") {
        args << "-f" << "matroska";
    }


    args << QDir::toNativeSeparators(outputFile);

    return args;
}

QString FileProcessor::findFFmpegExecutable()
{
    QProcess process;

    // Prefer explicit environment variable if provided
    QString envPath = qEnvironmentVariable("FFMPEG_PATH");
    if (!envPath.isEmpty()) {
        process.start(envPath, QStringList() << "-version");
        if (process.waitForStarted(3000) && process.waitForFinished(8000) && process.exitCode() == 0) {
            QString output = QString::fromUtf8(process.readAllStandardOutput() + process.readAllStandardError());
            parseAndLogFFmpegVersion(output);
            return envPath;
        }
    }

#ifdef Q_OS_WIN
    QString program = "ffmpeg.exe";
#else
    QString program = "ffmpeg";
#endif

    // Check if ffmpeg is available in PATH by running -version
    process.start(program, QStringList() << "-version");
    if (process.waitForStarted(3000) && process.waitForFinished(8000) && process.exitCode() == 0) {
        QString output = QString::fromUtf8(process.readAllStandardOutput() + process.readAllStandardError());
        parseAndLogFFmpegVersion(output);
        return program;
    }

    return QString(); // Not found
}

void FileProcessor::parseAndLogFFmpegVersion(const QString &versionOutput)
{
    // Parse FFmpeg version information from --version output
    // Example output: "ffmpeg version 4.4.2 Copyright (c) 2000-2021 the FFmpeg developers"
    
    QStringList lines = versionOutput.split('\n');
    if (lines.isEmpty()) {
        emit logMessage("FFmpeg found in PATH, but version information unavailable");
        return;
    }
    
    QString firstLine = lines.first();
    
    // Extract version number
    QRegularExpression versionRegex(R"(ffmpeg version ([\d\.\w-]+))");
    QRegularExpressionMatch versionMatch = versionRegex.match(firstLine);
    QString version = "Unknown";
    if (versionMatch.hasMatch()) {
        version = versionMatch.captured(1);
    }
    
    // Extract copyright year range
    QRegularExpression yearRegex(R"(Copyright \(c\) (\d{4})-(\d{4}))");
    // Search across all lines, not just the first
    QString joined = versionOutput;
    QRegularExpressionMatch yearMatch = yearRegex.match(joined);
    QString yearRange = "Unknown";
    if (yearMatch.hasMatch()) {
        QString startYear = yearMatch.captured(1);
        QString endYear = yearMatch.captured(2);
        yearRange = QString("%1-%2").arg(startYear).arg(endYear);
    }
    
    emit logMessage(QString("FFmpeg found in PATH - Version: %1, Release Years: %2")
                   .arg(version).arg(yearRange));
}
