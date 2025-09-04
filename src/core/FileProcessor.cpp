#include "FileProcessor.h"
#include "MuxingTask.h"
#include "../ui/MainWindow.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QProcess>
#include <QSettings>

QString FileProcessor::detectVideoFormatFromFileName(const QString &fileName)
{
    QMap<QString, QStringList> formatKeywords;
    formatKeywords["hevc"] = {"h265", "hevc"};
    formatKeywords["h264"] = {"h264", "avc"};
    formatKeywords["vp9"]  = {"vp9"};
    formatKeywords["av1"]  = {"av1"};

    QString lowerFileName = fileName.toLower();
    for (auto it = formatKeywords.constBegin(); it != formatKeywords.constEnd(); ++it) {
        const QStringList &keywords = it.value();
        for (const QString &keyword : keywords) {
            if (lowerFileName.contains(keyword)) {
                return it.key();
            }
        }
    }
    return QString();
}

QString FileProcessor::parsePixelFormat(const MediaInfo &mediaInfo) const
{
    if (!mediaInfo.colorSpace.isEmpty()) {
        if (mediaInfo.colorSpace.contains("yuv420p10le") || mediaInfo.colorSpace.contains("10bit") ||
            mediaInfo.bitDepth == "10") {
            return "yuv420p10le";
        }
        if (mediaInfo.colorSpace.contains("yuv420p")) {
            return "yuv420p";
        }
    }
    // Default value
    return (mediaInfo.bitDepth == "10") ? "yuv420p10le" : "yuv420p";
}

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
    // CRITICAL PATH: Main entry point for batch file processing
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

    qDeleteAll(m_taskQueue);
    m_taskQueue.clear();

    emit logMessage(QString("Starting to process %1 files...").arg(m_totalFiles));
    emit logMessage(QString("FFmpeg path: %1").arg(m_ffmpegPath.isEmpty() ? "Not found" : m_ffmpegPath));

    if (m_ffmpegPath.isEmpty()) {
        emit logMessage("[ERROR] FFmpeg executable not found. Please ensure FFmpeg is in PATH.");
        m_processing = false;
        return;
    }


    // CRITICAL PATH: Create processing tasks for each file in the batch
    for (int i = 0; i < m_files.size(); ++i) {
        const QString &inputFile = m_files[i];
        const MediaInfo mediaInfo = (i < m_mediaInfos.size()) ? m_mediaInfos[i] : MediaInfo();

        QString outputFile = generateOutputFilePath(inputFile, mediaInfo);


        MuxingTask *task = new MuxingTask(this);
        task->setFiles(inputFile, outputFile);

        // Build command based on processing mode (standard muxing vs BIN->YUV conversion)
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
    // CRITICAL PATH: Process files one by one from the queue
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

    emit fileProcessed(inputFile, success);

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
    // CRITICAL PATH: Build FFmpeg command with proper codec and format settings
    QStringList args;

    args << "-fflags" << "+genpts";

    // Only specify -framerate when duration is unknown (likely raw streams)
    if (mediaInfo.duration.isEmpty() || mediaInfo.duration.compare("Unknown", Qt::CaseInsensitive) == 0) {
        auto normalizeFpsStr = [](const QString &s) -> QString {
            QString text = s;
            QString src = text;
            // Try rational first
            QRegularExpression rat(R"((\d+)\s*/\s*(\d+))");
            QRegularExpressionMatch rm = rat.match(src);
            double fps = 0.0; bool ok = false;
            if (rm.hasMatch()) {
                bool ok1=false, ok2=false; double num = rm.captured(1).toDouble(&ok1); double den = rm.captured(2).toDouble(&ok2);
                if (ok1 && ok2 && den != 0.0) { fps = num/den; ok = true; }
            }
            if (!ok) {
                QRegularExpression numRe(R"((\d{1,3}(?:\.\d{1,3})?))");
                QRegularExpressionMatch nm = numRe.match(src);
                if (nm.hasMatch()) { fps = nm.captured(1).toDouble(&ok); }
            }
            if (!ok || fps < 1.0 || fps > 240.0) fps = 30.0;
            return QString::number(fps, 'f', (fabs(fps - (int)fps) < 1e-6) ? 0 : 3);
        };
        QString frameRate = normalizeFpsStr(mediaInfo.frameRate);
        args << "-framerate" << frameRate;
    }

    args << "-i" << QDir::toNativeSeparators(inputFile);

    // Determine if AV1 film grain is enabled
    bool enableFilmGrain = false; int filmGrainValue = 0;
    {
        QSettings settings; // Use same scope as UI
        enableFilmGrain = settings.value("filmgrainEnabled", false).toBool();
        filmGrainValue = settings.value("filmgrainValue", 25).toInt();
        if (filmGrainValue < 0) filmGrainValue = 0; if (filmGrainValue > 50) filmGrainValue = 50;
    }

    bool isAv1 = mediaInfo.videoCodec.contains("AV1", Qt::CaseInsensitive);

    if (enableFilmGrain && isAv1) {
        // Re-encode with libsvtav1 and apply film grain synthesis
        args << "-c:v" << "libsvtav1";
        args << "-crf" << "30"; // reasonable default
        args << "-preset" << "6";
        args << "-b:v" << "0";
        // Set pixel format based on bit depth
        QString pixelFormat = parsePixelFormat(mediaInfo);
        args << "-pix_fmt" << pixelFormat;
        args << "-svtav1-params" << QString("film-grain=%1").arg(filmGrainValue);
    } else {
        // Default: stream copy for video
        args << "-c:v" << "copy";
    }

    if (m_overwrite) {
        args << "-y";
    }

    if (format.toLower() == "mp4") {
        args << "-f" << "mp4";
        args << "-movflags" << "faststart";
    } else if (format.toLower() == "mkv") {
        args << "-f" << "matroska";
    }

    // CRITICAL PATH: HDR handling - preserve HDR metadata for proper color reproduction
    if (mediaInfo.isHdr) {
        // Default primaries/matrix to BT.2020 if unknown
        QString primaries = mediaInfo.colorPrimariesCode.isEmpty() ? "bt2020" : mediaInfo.colorPrimariesCode;
        QString matrix = mediaInfo.colorSpaceCode.isEmpty() ? "bt2020nc" : mediaInfo.colorSpaceCode;
        QString trc = (mediaInfo.hdrEotf == "HLG") ? "arib-std-b67" : "smpte2084"; // PQ default otherwise
        args << "-fps_mode" << "vfr"; // keep as in requirement example
        if (format.toLower() == "mp4") {
            args << "-movflags" << "faststart";
        }
        args << "-color_primaries" << primaries;
        args << "-color_trc" << trc;
        args << "-colorspace" << matrix;
    }

    args << QDir::toNativeSeparators(outputFile);

    return args;
}

QString FileProcessor::findFFmpegExecutable()
{
    // CRITICAL PATH: Locate FFmpeg executable - required for all operations
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

QString FileProcessor::generateOutputFilePath(const QString &inputFile, const MediaInfo &mediaInfo)
{
    // CRITICAL PATH: Generate output file paths with proper naming conventions
    QFileInfo inputInfo(inputFile);
    QString outputName;

    if (m_processingMode == "binToYuv") {
        QString baseName = inputInfo.completeBaseName();
        QString bitDepth = mediaInfo.bitDepth.isEmpty() ? "8" : mediaInfo.bitDepth;

        QString resolution = mediaInfo.resolution.isEmpty() ? "1920x1080" : mediaInfo.resolution;
        QRegularExpression resRegex(R"((\d+x\d+))");
        QRegularExpressionMatch resMatch = resRegex.match(resolution);
        if (resMatch.hasMatch()) {
            resolution = resMatch.captured(1);
        }

        QString frameRate = mediaInfo.frameRate;
        frameRate.remove(" fps", Qt::CaseInsensitive).remove("fps", Qt::CaseInsensitive);
        if (frameRate.isEmpty() || frameRate.contains("Unknown", Qt::CaseInsensitive)) frameRate = "30";

        QString colorFormat = parsePixelFormat(mediaInfo);

        outputName = QString("%1_%2bit_decoded_%3_%4fps_%5.yuv")
                         .arg(baseName).arg(bitDepth).arg(resolution).arg(frameRate).arg(colorFormat);
    } else { // "muxing" mode
        outputName = inputInfo.completeBaseName() + "_muxed." + m_outputFormat;
    }

    QString outputFile = QDir(m_outputFolder).absoluteFilePath(outputName);

    if (QFile::exists(outputFile) && !m_overwrite) {
        int counter = 1;
        QString baseName = inputInfo.completeBaseName();
        QString extension = QFileInfo(outputName).suffix();
        QString basePart = baseName + (m_processingMode == "binToYuv" ? "_decoded" : "_muxed");

        do {
            outputName = QString("%1_(%2).%3").arg(basePart).arg(counter++).arg(extension);
            outputFile = QDir(m_outputFolder).absoluteFilePath(outputName);
        } while (QFile::exists(outputFile));
    }

    return outputFile;
}

QStringList FileProcessor::buildBinToYuvCommand(const QString &inputFile, const QString &outputFile,
                                                const MediaInfo &mediaInfo)
{
    QStringList args;
    QString detectedFormat = detectVideoFormatFromFileName(QFileInfo(inputFile).fileName());
    QString pixelFormat = parsePixelFormat(mediaInfo);

    args << "-i" << QDir::toNativeSeparators(inputFile);
    args << "-c:v" << "rawvideo";
    args << "-pix_fmt" << pixelFormat;
    args << "-y";

    args << QDir::toNativeSeparators(outputFile);

    return args;
}
