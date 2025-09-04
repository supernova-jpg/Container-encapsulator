#ifndef MEDIAANALYZER_H
#define MEDIAANALYZER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQueue>

struct MediaInfo;

struct AnalysisTask {
    int index;
    QString filePath;
};

class MediaAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit MediaAnalyzer(QObject *parent = nullptr);
    ~MediaAnalyzer();

    void analyzeFile(int index, const QString &filePath);
    void analyzeFiles(const QStringList &files);
    void stop();
    
    bool isAnalyzing() const { return m_analyzing; }

signals:
    void analysisFinished(int index, const MediaInfo &info);
    void analysisError(int index, const QString &error);
    void allAnalysisFinished();

private slots:
    void processNextFile();
    void onProbeFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProbeError(QProcess::ProcessError error);

private:
    MediaInfo parseFFprobeOutput(const QString &output);
    QString formatDuration(double seconds);
    QString formatFileSize(qint64 size);
    QString findFFprobeExecutable();
    bool isRawStreamFile(const QString &filePath);
    MediaInfo createDefaultMediaInfo(const QString &filePath);
    QString getResolutionDescription(int width, int height);
    void parseAndLogFFprobeVersion(const QString &versionOutput);
    QString normalizeFpsFromText(const QString &text);
    QString extractFpsFromName(const QString &name);
    QString bitDepthFromPixelFormat(const QString &pixFmt);
    
    QProcess *m_probeProcess;
    QQueue<AnalysisTask> m_taskQueue;
    bool m_analyzing;
    QString m_ffprobePath;
    AnalysisTask m_currentTask;
};

#endif
