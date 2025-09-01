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

    // For raw streams, we need special handling
    if (mediaInfo.isRawStream || !mediaInfo.analyzed) {
        // Add input format detection for raw streams
        QString extension = QFileInfo(inputFile).suffix().toLower();
        QString fileName = QFileInfo(inputFile).fileName().toLower();
        
        // Detect codec format
        if (extension == "bin") {
            // For .bin files, try to detect codec from filename
            if (fileName.contains("hevc") || fileName.contains("h265") || fileName.contains("265")) {
                args << "-f" << "hevc";
            } else if (fileName.contains("h264") || fileName.contains("avc") || fileName.contains("264")) {
                args << "-f" << "h264";
            } else {
                // Default to HEVC for unknown .bin files (most common in testing)
                args << "-f" << "hevc";
            }
        } else if (extension == "h264" || extension == "264") {
            args << "-f" << "h264";
        } else if (extension == "h265" || extension == "265" || extension == "hevc") {
            args << "-f" << "hevc";
        }
        
        // Check for pixel format hints in filename
        if (fileName.contains("p010") || fileName.contains("10bit") || fileName.contains("10-bit")) {
            // For 10-bit content, specify pixel format
            if (fileName.contains("p010")) {
                args << "-pix_fmt" << "p010le";
            }
        }

        // Add framerate before input file for raw streams
        if (!mediaInfo.frameRate.isEmpty() && !mediaInfo.frameRate.contains("Unknown")) {
            QString frameRate = mediaInfo.frameRate;
            frameRate.remove(" fps").remove("fps");
            bool ok;
            double fps = frameRate.toDouble(&ok);

            if (ok && fps > 0) {
                args << "-r" << QString::number(fps);
            }
        }
        
        // Extract resolution from filename if available
        QRegExp resolutionRegex("(\\d{3,4})x(\\d{3,4})");
        if (fileName.contains(resolutionRegex)) {
            QString resolution = resolutionRegex.cap(0);
            args << "-s" << resolution;
        }
    }

    // Always use genpts for better timestamp handling
    args << "-fflags" << "+genpts";
    
    // Input file
    args << "-i" << QDir::toNativeSeparators(inputFile);

    // Copy video codec
    args << "-c:v" << "copy";
    
    // Copy all streams (video, audio, subtitles)
    args << "-c:a" << "copy";
    args << "-c:s" << "copy";

    if (m_overwrite) {
        args << "-y";
    }

    if (format.toLower() == "mp4") {
        args << "-f" << "mp4";
        args << "-movflags" << "+faststart";
    } else if (format.toLower() == "mkv") {
        args << "-f" << "matroska";
    }

    args << QDir::toNativeSeparators(outputFile);

    return args;
}

QString FileProcessor::findFFmpegExecutable()
{
    QProcess process;
    
    QString program = "ffmpeg.exe";

    
    // Check if ffmpeg is available in PATH by running -version
    process.start(program, QStringList() << "-version");
    if (process.waitForStarted(3000) && process.waitForFinished(8000)) {
        if (process.exitCode() == 0) {
            // Parse version output to extract version and year
            QString output = QString::fromUtf8(process.readAllStandardOutput());
            parseAndLogFFmpegVersion(output);
            return program;
        }
    }
    
    return QString(); // Not found in PATH
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
