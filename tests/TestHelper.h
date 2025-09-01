#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>

/**
 * TestHelper - Utility class for creating test files and mock FFmpeg output
 */
class TestHelper : public QObject
{
    Q_OBJECT

public:
    explicit TestHelper(QObject *parent = nullptr);
    
    // Test file creation
    QString createTestVideoFile(const QString &filename, qint64 size = 1024 * 1024);
    QString createTestH264File(const QString &filename, qint64 size = 500 * 1024);
    QString createTestH265File(const QString &filename, qint64 size = 750 * 1024);
    QStringList createTestFileCollection(int count = 5);
    
    // Mock FFmpeg executable creation
    QString createMockFFmpeg();
    QString createMockFFprobe();
    
    // Mock FFprobe output generation
    QString generateFFprobeOutput(const QString &videoCodec = "h264", 
                                const QString &resolution = "1920x1080",
                                const QString &frameRate = "30.000",
                                const QString &bitDepth = "8",
                                const QString &colorSpace = "bt709",
                                double duration = 120.5,
                                qint64 fileSize = 1048576);
    
    // System theme testing
    void simulateWindowsLightTheme();
    void simulateWindowsDarkTheme();
    void simulateWindowsHighContrastTheme();
    
    // FFmpeg command validation
    bool validateFFmpegCommand(const QString &command, const QString &inputFile, 
                             const QString &outputFile, const QString &format);
    bool commandContainsMetadata(const QString &command);
    
    // Test cleanup
    void cleanup();
    
    // Getters
    QString getTempDir() const { return m_tempDir->path(); }
    QStringList getCreatedFiles() const { return m_createdFiles; }

private:
    QString createMockExecutable(const QString &name, const QString &script);
    
    QTemporaryDir *m_tempDir;
    QStringList m_createdFiles;
    QString m_mockFFmpegPath;
    QString m_mockFFprobePath;
};

/**
 * MediaInfoMatcher - Helper class for validating MediaInfo structures
 */
class MediaInfoMatcher
{
public:
    static bool matches(const struct MediaInfo &info, const QString &expectedCodec,
                       const QString &expectedResolution, const QString &expectedFrameRate,
                       const QString &expectedBitDepth, const QString &expectedColorSpace);
    
    static bool hasValidMetadata(const struct MediaInfo &info);
    static bool isAnalyzed(const struct MediaInfo &info);
};

/**
 * UIEventSimulator - Helper for simulating UI interactions
 */
class UIEventSimulator
{
public:
    static void addFiles(class MainWindow *window, const QStringList &files);
    static void dragAndDropFiles(class MainWindow *window, const QStringList &files);
    static void selectOutputFormat(class MainWindow *window, const QString &format);
    static void setOutputFolder(class MainWindow *window, const QString &folder);
    static void startProcessing(class MainWindow *window);
    static void stopProcessing(class MainWindow *window);
    static void clearFiles(class MainWindow *window);
};

// Macros for common test patterns
#define VERIFY_FFMPEG_COMMAND(command, input, output, format) \
    QVERIFY2(TestHelper::validateFFmpegCommand(command, input, output, format), \
             QString("Invalid FFmpeg command: %1").arg(command).toLocal8Bit().data())

#define VERIFY_METADATA_PRESENT(command) \
    QVERIFY2(TestHelper::commandContainsMetadata(command), \
             QString("FFmpeg command missing metadata: %1").arg(command).toLocal8Bit().data())

#define WAIT_FOR_SIGNAL(object, signal, timeout) \
    do { \
        QSignalSpy spy(object, signal); \
        QVERIFY(spy.wait(timeout)); \
    } while(0)

#endif // TESTHELPER_H