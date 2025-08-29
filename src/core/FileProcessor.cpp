#include "FileProcessor.h"
#include "MuxingTask.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

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
                               const QString &format, bool overwrite)
{
    if (m_processing) {
        emit logMessage("Already processing files. Stop current operation first.");
        return;
    }
    
    m_files = files;
    m_outputFolder = outputFolder;
    m_outputFormat = format;
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
        emit error("FFmpeg executable not found. Please ensure FFmpeg is installed and in PATH.");
        m_processing = false;
        return;
    }
    
    // Create tasks for all files
    foreach (const QString &inputFile, m_files) {
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
        
        QString command = buildFFmpegCommand(inputFile, outputFile, m_outputFormat);
        task->setFFmpegCommand(command);
        
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
        emit logMessage("All files processed successfully!");
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
        emit logMessage(QString("✗ Failed to process: %1 - %2")
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

QString FileProcessor::buildFFmpegCommand(const QString &inputFile, const QString &outputFile, const QString &format)
{
    QStringList args;
    
    // Input file
    args << "-i" << QDir::toNativeSeparators(inputFile);
    
    // Copy streams (no re-encoding)
    args << "-c" << "copy";
    
    // Map all streams
    args << "-map" << "0";
    
    // Overwrite output file if it exists
    if (m_overwrite) {
        args << "-y";
    }
    
    // Output format specific settings
    if (format.toLower() == "mp4") {
        args << "-f" << "mp4";
        args << "-movflags" << "faststart"; // Move metadata to beginning for faster web streaming
    } else if (format.toLower() == "mkv") {
        args << "-f" << "matroska";
    } else if (format.toLower() == "mov") {
        args << "-f" << "mov";
        args << "-movflags" << "faststart";
    } else if (format.toLower() == "webm") {
        args << "-f" << "webm";
    } else if (format.toLower() == "ts") {
        args << "-f" << "mpegts";
    }
    
    // Output file
    args << QDir::toNativeSeparators(outputFile);
    
    return QString("\"%1\" %2").arg(m_ffmpegPath).arg(args.join(" "));
}

QString FileProcessor::findFFmpegExecutable()
{
    // Common locations to search for FFmpeg
    QStringList possiblePaths;
    
#ifdef Q_OS_WIN
    possiblePaths << "ffmpeg.exe"
                  << "C:/ffmpeg/bin/ffmpeg.exe"
                  << "C:/Program Files/ffmpeg/bin/ffmpeg.exe"
                  << "C:/Program Files (x86)/ffmpeg/bin/ffmpeg.exe";
#else
    possiblePaths << "ffmpeg"
                  << "/usr/bin/ffmpeg"
                  << "/usr/local/bin/ffmpeg"
                  << "/opt/homebrew/bin/ffmpeg";
#endif
    
    // First, try to find in PATH
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