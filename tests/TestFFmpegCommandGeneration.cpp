#include "TestFFmpegCommandGeneration.h"
#include <QSignalSpy>
#include <QTimer>
#include <QApplication>

void TestFFmpegCommandGeneration::initTestCase()
{
    m_helper = new TestHelper(this);
    
    // Create test files
    m_testVideoFile = m_helper->createTestVideoFile("test_video.mp4", 10 * 1024 * 1024);
    m_testH264File = m_helper->createTestH264File("raw_stream.h264", 5 * 1024 * 1024);
    m_testH265File = m_helper->createTestH265File("raw_hevc.h265", 7 * 1024 * 1024);
    
    // Setup mock FFmpeg environment
    QString mockFFmpeg = m_helper->createMockFFmpeg();
    QString mockFFprobe = m_helper->createMockFFprobe();
    
    // Set environment for testing
    qputenv("FFMPEG_PATH", mockFFmpeg.toLocal8Bit());
    qputenv("FFPROBE_PATH", mockFFprobe.toLocal8Bit());
}

void TestFFmpegCommandGeneration::cleanupTestCase()
{
    m_helper->cleanup();
    delete m_helper;
}

void TestFFmpegCommandGeneration::init()
{
    m_processor = new FileProcessor(this);
    m_analyzer = new MediaAnalyzer(this);
    m_mainWindow = new MainWindow();
}

void TestFFmpegCommandGeneration::cleanup()
{
    delete m_mainWindow;
    delete m_analyzer;
    delete m_processor;
}

void TestFFmpegCommandGeneration::testBasicCommandGeneration()
{
    // Test basic FFmpeg command structure
    QString inputFile = m_testVideoFile;
    QString outputFile = m_helper->getTempDir() + "/output.mp4";
    QString format = "mp4";
    
    // Access private method through reflection or friend class
    // For now, test through FileProcessor public interface
    QStringList files;
    files << inputFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), format, mediaInfos, true);
    
    // Wait for log messages
    QVERIFY(logSpy.wait(5000));
    
    // Verify command was logged
    bool commandLogged = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            commandLogged = true;
            QVERIFY(message.contains("-i"));
            QVERIFY(message.contains(inputFile));
            QVERIFY(message.contains(outputFile));
            break;
        }
    }
    QVERIFY2(commandLogged, "FFmpeg command was not logged");
}

void TestFFmpegCommandGeneration::testCommandWithMetadata()
{
    // Test that FFmpeg commands include metadata parameters
    MediaInfo testInfo;
    testInfo.videoCodec = "H.264";
    testInfo.resolution = "1920x1080";
    testInfo.frameRate = "30.000 fps";
    testInfo.bitDepth = "8 bit";
    testInfo.colorSpace = "Rec. 709";
    testInfo.analyzed = true;
    
    createTestFileWithMetadata("test_with_metadata.mp4", testInfo);
    
    QStringList files;
    files << m_testVideoFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mp4", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    // Look for metadata parameters in the command
    bool metadataFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            // The command should include metadata parameters
            // This is the core bug we're testing for
            QVERIFY2(m_helper->commandContainsMetadata(message),
                    QString("Command missing metadata: %1").arg(message).toLocal8Bit().data());
            metadataFound = true;
            break;
        }
    }
    QVERIFY2(metadataFound, "No FFmpeg command found in logs");
}

void TestFFmpegCommandGeneration::testCommandWithH264Metadata()
{
    // Test H.264 specific metadata inclusion
    MediaInfo h264Info;
    h264Info.videoCodec = "H.264";
    h264Info.resolution = "1920x1080";
    h264Info.frameRate = "29.970 fps";
    h264Info.bitDepth = "8 bit";
    h264Info.colorSpace = "Rec. 709";
    h264Info.analyzed = true;
    
    QStringList files;
    files << m_testH264File;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mp4", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    // Verify H.264 specific parameters
    bool h264CommandFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            // Should contain resolution and frame rate for H.264
            verifyMetadataInCommand(message, h264Info);
            h264CommandFound = true;
            break;
        }
    }
    QVERIFY(h264CommandFound);
}

void TestFFmpegCommandGeneration::testCommandWithH265Metadata()
{
    // Test H.265/HEVC specific metadata inclusion
    MediaInfo h265Info;
    h265Info.videoCodec = "H.265/HEVC";
    h265Info.resolution = "3840x2160";
    h265Info.frameRate = "24.000 fps";
    h265Info.bitDepth = "10 bit";
    h265Info.colorSpace = "Rec. 2020";
    h265Info.analyzed = true;
    
    QStringList files;
    files << m_testH265File;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mkv", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    // Verify H.265 specific parameters
    bool h265CommandFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            verifyMetadataInCommand(message, h265Info);
            h265CommandFound = true;
            break;
        }
    }
    QVERIFY(h265CommandFound);
}

void TestFFmpegCommandGeneration::testCommandWithHDRMetadata()
{
    // Test HDR metadata inclusion (critical for 10-bit/HDR workflows)
    MediaInfo hdrInfo;
    hdrInfo.videoCodec = "H.265/HEVC";
    hdrInfo.resolution = "3840x2160";
    hdrInfo.frameRate = "24.000 fps";
    hdrInfo.bitDepth = "10 bit";
    hdrInfo.colorSpace = "Rec. 2020";
    hdrInfo.analyzed = true;
    
    QString hdrFile = m_helper->createTestVideoFile("hdr_content.mkv", 15 * 1024 * 1024);
    
    QStringList files;
    files << hdrFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mkv", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    // Verify HDR-specific parameters are included
    bool hdrCommandFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            // HDR content needs specific colorspace and bit depth parameters
            QVERIFY2(message.contains("2020") || message.contains("bt2020"),
                    "HDR command missing Rec. 2020 colorspace");
            QVERIFY2(message.contains("10") || message.contains("p010"),
                    "HDR command missing 10-bit information");
            hdrCommandFound = true;
            break;
        }
    }
    QVERIFY(hdrCommandFound);
}

void TestFFmpegCommandGeneration::testResolutionMetadataInCommand()
{
    // Test that resolution metadata is properly included in commands
    MediaInfo info;
    info.resolution = "3840x2160";
    info.analyzed = true;
    
    QStringList files;
    files << m_testVideoFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mp4", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    bool resolutionFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            // Resolution should be included as metadata or filter parameter
            QString extracted = extractMetadataFromCommand(message, "resolution");
            QVERIFY2(!extracted.isEmpty() || message.contains("3840x2160"),
                    "Resolution metadata missing from command");
            resolutionFound = true;
            break;
        }
    }
    QVERIFY(resolutionFound);
}

void TestFFmpegCommandGeneration::testFrameRateMetadataInCommand()
{
    // Test frame rate metadata inclusion
    MediaInfo info;
    info.frameRate = "23.976 fps";
    info.analyzed = true;
    
    QStringList files;
    files << m_testVideoFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mp4", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    bool frameRateFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            // Frame rate should be included
            QString extracted = extractMetadataFromCommand(message, "framerate");
            QVERIFY2(!extracted.isEmpty() || message.contains("23.976"),
                    "Frame rate metadata missing from command");
            frameRateFound = true;
            break;
        }
    }
    QVERIFY(frameRateFound);
}

void TestFFmpegCommandGeneration::testBitDepthMetadataInCommand()
{
    // Test bit depth metadata inclusion (critical for quality preservation)
    MediaInfo info;
    info.bitDepth = "10 bit";
    info.analyzed = true;
    
    QStringList files;
    files << m_testVideoFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mkv", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    bool bitDepthFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            // Bit depth should influence pixel format selection
            QVERIFY2(message.contains("10") || message.contains("p010") || message.contains("yuv420p10le"),
                    "Bit depth metadata not reflected in command");
            bitDepthFound = true;
            break;
        }
    }
    QVERIFY(bitDepthFound);
}

void TestFFmpegCommandGeneration::testColorSpaceMetadataInCommand()
{
    // Test color space metadata inclusion
    MediaInfo info;
    info.colorSpace = "Rec. 2020";
    info.analyzed = true;
    
    QStringList files;
    files << m_testVideoFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mkv", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    bool colorSpaceFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            // Color space should be preserved
            QVERIFY2(message.contains("2020") || message.contains("bt2020"),
                    "Color space metadata missing from command");
            colorSpaceFound = true;
            break;
        }
    }
    QVERIFY(colorSpaceFound);
}

void TestFFmpegCommandGeneration::testCompleteMetadataIntegration()
{
    // Test that all metadata components work together
    MediaInfo completeInfo;
    completeInfo.videoCodec = "H.265/HEVC";
    completeInfo.resolution = "3840x2160";
    completeInfo.frameRate = "60.000 fps";
    completeInfo.bitDepth = "10 bit";
    completeInfo.colorSpace = "Rec. 2020";
    completeInfo.analyzed = true;
    
    QStringList files;
    files << m_testVideoFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mkv", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    bool completeCommandFound = false;
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            // All metadata should be present and correctly integrated
            verifyMetadataInCommand(message, completeInfo);
            completeCommandFound = true;
            break;
        }
    }
    QVERIFY(completeCommandFound);
}

void TestFFmpegCommandGeneration::testMetadataMissingBugReproduction()
{
    // This test reproduces the original bug: FFmpeg commands without metadata
    // Should FAIL with current code, PASS after fix
    
    QStringList files;
    files << m_testVideoFile;
    
    QSignalSpy logSpy(m_processor, &FileProcessor::logMessage);
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mp4", mediaInfos, true);
    
    QVERIFY(logSpy.wait(5000));
    
    bool commandFound = false;
    bool hasMetadata = false;
    
    for (const QList<QVariant> &arguments : logSpy) {
        QString message = arguments.first().toString();
        if (message.contains("FFmpeg:")) {
            commandFound = true;
            hasMetadata = m_helper->commandContainsMetadata(message);
            
            // This assertion should fail with the current buggy code
            // but pass after the fix is applied
            QVERIFY2(hasMetadata, 
                    QString("BUG REPRODUCED: FFmpeg command missing metadata: %1")
                    .arg(message).toLocal8Bit().data());
            break;
        }
    }
    
    QVERIFY2(commandFound, "No FFmpeg command was generated");
}

void TestFFmpegCommandGeneration::testProcessingFailureDueToMissingMetadata()
{
    // Test that missing metadata causes processing failures
    // This simulates the real-world scenario where missing metadata leads to errors
    
    QString problemFile = m_helper->createTestH264File("problem_file.h264", 1024 * 1024);
    
    QStringList files;
    files << problemFile;
    
    QSignalSpy errorSpy(m_processor, &FileProcessor::error);
    QSignalSpy finishedSpy(m_processor, &FileProcessor::finished);
    
    QVector<MediaInfo> mediaInfos; // Empty for test
    m_processor->processFiles(files, m_helper->getTempDir(), "mp4", mediaInfos, true);
    
    // Wait for processing to complete
    QVERIFY(finishedSpy.wait(10000));
    
    // With the current bug, this might generate errors due to missing metadata
    // After the fix, this should process successfully
    if (errorSpy.count() > 0) {
        QString errorMessage = errorSpy.first().first().toString();
        QWARN(QString("Processing failed (expected with current bug): %1")
              .arg(errorMessage).toLocal8Bit().data());
    }
}

// Helper methods
void TestFFmpegCommandGeneration::verifyMetadataInCommand(const QString &command, const MediaInfo &mediaInfo)
{
    if (!mediaInfo.resolution.isEmpty() && mediaInfo.resolution != "Unknown") {
        // Extract resolution values
        QString resolution = mediaInfo.resolution;
        resolution.remove(QRegExp("[()].*")); // Remove format descriptions
        QStringList parts = resolution.split("x");
        if (parts.size() == 2) {
            QString width = parts[0].trimmed();
            QString height = parts[1].trimmed();
            QVERIFY2(command.contains(width) && command.contains(height),
                    QString("Resolution %1 not found in command").arg(mediaInfo.resolution).toLocal8Bit().data());
        }
    }
    
    if (!mediaInfo.frameRate.isEmpty() && mediaInfo.frameRate != "Unknown") {
        QString frameRate = mediaInfo.frameRate;
        frameRate.remove(" fps"); // Remove fps suffix
        QVERIFY2(command.contains(frameRate),
                QString("Frame rate %1 not found in command").arg(mediaInfo.frameRate).toLocal8Bit().data());
    }
    
    if (!mediaInfo.colorSpace.isEmpty() && mediaInfo.colorSpace != "Unknown") {
        if (mediaInfo.colorSpace.contains("2020")) {
            QVERIFY2(command.contains("2020") || command.contains("bt2020"),
                    "Rec. 2020 color space not found in command");
        } else if (mediaInfo.colorSpace.contains("709")) {
            QVERIFY2(command.contains("709") || command.contains("bt709"),
                    "Rec. 709 color space not found in command");
        }
    }
}

void TestFFmpegCommandGeneration::createTestFileWithMetadata(const QString &filename, const MediaInfo &info)
{
    // Create a test file and associate it with specific metadata
    // This would be used with a mock MediaAnalyzer that returns the specified info
    QString filePath = m_helper->getTempDir() + "/" + filename;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QByteArray(1024 * 1024, 'x')); // 1MB dummy data
        file.close();
    }
}

QString TestFFmpegCommandGeneration::extractMetadataFromCommand(const QString &command, const QString &parameter)
{
    // Helper to extract specific metadata parameters from FFmpeg commands
    QRegExp regex(QString("-%1\\s+(\\S+)").arg(parameter));
    if (regex.indexIn(command) != -1) {
        return regex.cap(1);
    }
    return QString();
}

// Additional test methods for other scenarios...
void TestFFmpegCommandGeneration::testMP4FormatWithMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testMKVFormatWithMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testMOVFormatWithMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testWebMFormatWithMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testTSFormatWithMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testRawH264StreamCommand() { /* Implementation */ }
void TestFFmpegCommandGeneration::testRawH265StreamCommand() { /* Implementation */ }
void TestFFmpegCommandGeneration::testRawStreamWithManualMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testCommandGenerationWithMissingMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testCommandGenerationWithInvalidMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testCommandGenerationWithPartialMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testCommandAfterSuccessfulAnalysis() { /* Implementation */ }
void TestFFmpegCommandGeneration::testCommandAfterFailedAnalysisWithDefaults() { /* Implementation */ }
void TestFFmpegCommandGeneration::testCommandWithUserEditedMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testHDR10MetadataIntegration() { /* Implementation */ }
void TestFFmpegCommandGeneration::testDolbyVisionMetadataIntegration() { /* Implementation */ }
void TestFFmpegCommandGeneration::test4KUHDMetadataIntegration() { /* Implementation */ }
void TestFFmpegCommandGeneration::test8KUHDMetadataIntegration() { /* Implementation */ }
void TestFFmpegCommandGeneration::testHighFrameRateMetadata() { /* Implementation */ }
void TestFFmpegCommandGeneration::testVariableFrameRateHandling() { /* Implementation */ }