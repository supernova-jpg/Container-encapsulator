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
                               bool overwrite, const QString &processingMode)
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
    m_processingMode = processingMode;
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
        QString outputName;
        QString outputFile;
        
        MediaInfo mediaInfo = (i < m_mediaInfos.size()) ? m_mediaInfos[i] : MediaInfo();
        
        if (m_processingMode == "binToYuv") {
            // For BIN->YUV conversion, use YUV naming convention
            // Format: {sequence}_{bitDepth}bit_{scene}_{resolution}_{frameRate}fps_{colorFormat}.yuv
            QString sequence = inputInfo.completeBaseName(); // Use filename as sequence
            QString bitDepth = mediaInfo.bitDepth.isEmpty() ? "8" : mediaInfo.bitDepth;
            QString scene = "converted"; // Default scene name
            QString resolution = mediaInfo.resolution.isEmpty() ? "1920x1080" : mediaInfo.resolution;
            QString frameRate = mediaInfo.frameRate;
            frameRate.remove(" fps").remove("fps");
            if (frameRate.isEmpty()) frameRate = "25";
            QString colorFormat = "yuv420p";
            
            outputName = QString("%1_%2bit_%3_%4_%5fps_%6.yuv")
                        .arg(sequence).arg(bitDepth).arg(scene)
                        .arg(resolution).arg(frameRate).arg(colorFormat);
        } else {
            // Standard muxing output name
            outputName = inputInfo.completeBaseName() + "_muxed." + m_outputFormat;
        }
        
        outputFile = QDir(m_outputFolder).absoluteFilePath(outputName);
        
        // Check if output file exists and handle accordingly
        if (QFile::exists(outputFile) && !m_overwrite) {
            // Auto-rename
            int counter = 1;
            QString baseName = (m_processingMode == "binToYuv") 
                              ? inputInfo.completeBaseName() + "_converted"
                              : inputInfo.completeBaseName() + "_muxed";
            QString extension = (m_processingMode == "binToYuv") ? "yuv" : m_outputFormat;
            do {
                outputName = QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension);
                outputFile = QDir(m_outputFolder).absoluteFilePath(outputName);
                counter++;
            } while (QFile::exists(outputFile));
        }
        
        MuxingTask *task = new MuxingTask(this);
        task->setFiles(inputFile, outputFile);

        QStringList commandArgs;
        if (m_processingMode == "binToYuv") {
            commandArgs = buildBinToYuvCommand(inputFile, outputFile, mediaInfo);
        } else {
            commandArgs = buildFFmpegCommand(inputFile, outputFile, m_outputFormat, mediaInfo);
        }
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
    if (mediaInfo.isRawStream || !mediaInfo.analyzed) {

        if (!mediaInfo.frameRate.isEmpty() && !mediaInfo.frameRate.contains("Unknown")) {
            QString frameRate = mediaInfo.frameRate;
            frameRate.remove(" fps").remove("fps");
            bool ok;
            double fps = frameRate.toDouble(&ok);

            if (ok && fps > 0) {
                args << "-framerate" << QString::number(fps);
            }
        }
    }

    args << "-i" << QDir::toNativeSeparators(inputFile);

    args << "-c:v" << "copy";

    if (m_overwrite) {
        args << "-y";
    }

    if (format.toLower() == "mp4") {
        args << "-f" << "mp4";
        args << "-movflags" << "faststart";
    } else if (format.toLower() == "mkv") {
        args << "-f" << "matroska";
    }


    args << QDir::toNativeSeparators(outputFile);

    return args;
}

QString FileProcessor::findFFmpegExecutable()
{
    QProcess process;
    
    // Try different possible names for ffmpeg
    QStringList candidates;
#ifdef Q_OS_WIN
    candidates << "ffmpeg" << "ffmpeg.exe";
#else
    candidates << "ffmpeg";
#endif
    
    for (const QString &program : candidates) {
        // Check if ffmpeg is available in PATH by running -version
        process.start(program, QStringList() << "-version");
        if (process.waitForStarted(5000) && process.waitForFinished(5000)) {
            if (process.exitCode() == 0) {
                QString output = QString::fromUtf8(process.readAllStandardOutput());
                // Verify it's actually FFmpeg by checking the output
                if (output.contains("ffmpeg version")) {
                    parseAndLogFFmpegVersion(output);
                    return program;
                }
            }
        }
        
        // Reset process for next attempt
        process.kill();
        process.waitForFinished(1000);
    }
    
    return QString(); // Not found in PATH
}

void FileProcessor::parseAndLogFFmpegVersion(const QString &versionOutput)
{
    // Parse FFmpeg version information from -version output
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
    QRegularExpressionMatch yearMatch = yearRegex.match(firstLine);
    QString yearRange = "Unknown";
    if (yearMatch.hasMatch()) {
        QString startYear = yearMatch.captured(1);
        QString endYear = yearMatch.captured(2);
        yearRange = QString("%1-%2").arg(startYear).arg(endYear);
    }
    
    emit logMessage(QString("FFmpeg found in PATH - Version: %1, Release Years: %2")
                   .arg(version).arg(yearRange));
}

QStringList FileProcessor::buildBinToYuvCommand(const QString &inputFile, const QString &outputFile,
                                               const MediaInfo &mediaInfo)
{
    QStringList args;
    
    // Input format settings for raw binary file
    args << "-f" << "rawvideo";
    
    // Parse pixel format from mediaInfo or use default
    QString pixelFormat = "yuv420p"; // default
    if (!mediaInfo.colorSpace.isEmpty()) {
        if (mediaInfo.colorSpace.contains("yuv420p10le") || mediaInfo.colorSpace.contains("10bit")) {
            pixelFormat = "yuv420p10le";
        } else if (mediaInfo.colorSpace.contains("yuv420p")) {
            pixelFormat = "yuv420p";
        }
    }
    args << "-pix_fmt" << pixelFormat;
    
    // Parse resolution from mediaInfo or use default
    QString resolution = "1920x1080"; // default
    if (!mediaInfo.resolution.isEmpty()) {
        resolution = mediaInfo.resolution;
        // Clean up resolution format (remove extra text)
        QRegularExpression resRegex(R"((\d+)x(\d+))");
        QRegularExpressionMatch match = resRegex.match(resolution);
        if (match.hasMatch()) {
            resolution = QString("%1x%2").arg(match.captured(1)).arg(match.captured(2));
        }
    }
    args << "-s" << resolution;
    
    // Parse frame rate from mediaInfo or use default
    QString frameRate = "25"; // default
    if (!mediaInfo.frameRate.isEmpty() && !mediaInfo.frameRate.contains("Unknown")) {
        QString fps = mediaInfo.frameRate;
        fps.remove(" fps").remove("fps");
        bool ok;
        double fpsValue = fps.toDouble(&ok);
        if (ok && fpsValue > 0) {
            frameRate = QString::number(fpsValue);
        }
    }
    args << "-r" << frameRate;
    
    // Input file
    args << "-i" << QDir::toNativeSeparators(inputFile);
    
    // Output settings for YUV file
    args << "-c:v" << "rawvideo";
    args << "-pix_fmt" << pixelFormat;
    
    // Overwrite if needed
    if (m_overwrite) {
        args << "-y";
    }
    
    // Output file
    args << QDir::toNativeSeparators(outputFile);
    
    return args;
}
