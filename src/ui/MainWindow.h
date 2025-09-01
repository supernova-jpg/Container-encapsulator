#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QComboBox>
#include <QCheckBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FileProcessor;
class MuxingTask;
class MediaAnalyzer;

struct MediaInfo {
    QString videoCodec;
    QString audioCodec;
    QString resolution;
    QString frameRate;
    QString duration;
    QString fileSize;
    QString bitrate;
    QString bitDepth;
    QString colorSpace;
    bool isRawStream = false;
    bool analyzed = false;
};

enum class LogLevel {
    Info,
    Warning,
    Error
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    // File operations
    void addFiles();
    void addFolder();
    void removeSelected();
    void clearAll();
    void analyzeFiles();
    
    // Settings
    void browseOutputFolder();
    void onFormatChanged();
    void onCompatibilityToggled();
    
    // Processing
    void startProcessing();
    void stopProcessing();
    void onTaskProgress(int current, int total, const QString &currentFile);
    void onTaskFinished();
    
    // Logging
    void onLogMessage(const QString &message, LogLevel level = LogLevel::Info);
    void clearLog();
    void onLogFilterChanged();
    
    // Media analysis
    void onMediaAnalysisFinished(int index, const MediaInfo &info);
    void onMediaAnalysisError(int index, const QString &error);
    
    // Table interaction
    void onTableCellChanged(int row, int column);
    void onTableItemDoubleClicked(int row, int column);
    
    // Environment setup
    void checkFFmpegEnvironment();
    void logFFmpegEnvironmentDetails();

private:
    void setupConnections();
    void updateFileTable();
    void addFilesToTable(const QStringList &files);
    bool isVideoFile(const QString &filePath);
    QString getOutputFormat() const;
    QString getOutputFileName(const QString &inputFile) const;
    QString formatFileSize(qint64 size);
    void logMessage(const QString &message, LogLevel level = LogLevel::Info);
    bool shouldShowLogLevel(LogLevel level);
    void updateTableRowStatus(int row, const QString &status);
    void showCompatibilityWarning(const QString &codec, const QString &container);
    
    // Editable table functionality
    void setupEditableCell(int row, int column, const QString &currentValue, const QStringList &options);
    QStringList getVideoCodecOptions();
    QStringList getResolutionPresets();
    QStringList getFrameRateOptions();
    QStringList getBitDepthOptions();
    QStringList getColorSpaceOptions();
    
    Ui::MainWindow *ui;
    
    // Data
    QStringList m_files;
    QVector<MediaInfo> m_mediaInfos;
    FileProcessor *m_processor;
    MediaAnalyzer *m_analyzer;
    bool m_processing;
    
    // UI state
    bool m_showInfo = true;
    bool m_showWarning = true;
    bool m_showError = true;
};

// Custom combo box delegate for raw stream codec selection
class CodecComboDelegate : public QObject
{
    Q_OBJECT
public:
    explicit CodecComboDelegate(QObject *parent = nullptr);
    QComboBox* createComboBox(QWidget *parent);
    static QStringList getVideoCodecs();
    static QStringList getAudioCodecs();
};

#endif // MAINWINDOW_H