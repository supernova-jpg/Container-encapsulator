// MuxingTask.h

#ifndef MUXINGTASK_H
#define MUXINGTASK_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QElapsedTimer>

class MuxingTask : public QObject
{
    Q_OBJECT

public:
    explicit MuxingTask(QObject *parent = nullptr);
    ~MuxingTask();

    void setFiles(const QString &inputFile, const QString &outputFile);
    void setCommandAndArgs(const QString &program, const QStringList &args);

    void start();
    void stop();

    bool isRunning() const;
    QString getInputFile() const { return m_inputFile; }
    QString getOutputFile() const { return m_outputFile; }

signals:
    void finished(bool success, const QString &message);
    void logMessage(const QString &message);
    void progress(int percentage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessReadyRead();
    void checkProgress();

private:
    void parseFFmpegOutput(const QString &output);
    QString formatDuration(qint64 seconds);

    QProcess *m_process;
    QTimer *m_progressTimer;
    QElapsedTimer m_elapsedTimer;

    QString m_inputFile;
    QString m_outputFile;

    QString m_program;
    QStringList m_arguments;

    QString m_accumulatedOutput;
    qint64 m_totalDuration;
    qint64 m_currentTime;
    bool m_durationParsed;
};

#endif // MUXINGTASK_H
