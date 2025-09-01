#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QStringList>
#include <QQueue>
#include <QFileInfo>
#include <QMap>
#include "../ui/MainWindow.h" // For MediaInfo struct

class MuxingTask;

class FileProcessor : public QObject
{
    Q_OBJECT

public:
    explicit FileProcessor(QObject *parent = nullptr);
    ~FileProcessor();

    void processFiles(const QStringList &files, const QString &outputFolder, 
                     const QString &format, bool overwrite = false);
    void setMediaInfos(const QList<MediaInfo> &mediaInfos) { m_mediaInfos = mediaInfos; }
    void stop();
    
    bool isProcessing() const { return m_processing; }

signals:
    void progress(int current, int total, const QString &currentFile);
    void finished();
    void logMessage(const QString &message);
    void error(const QString &message);

private slots:
    void processNextFile();
    void onTaskFinished(bool success, const QString &message);

private:
    void startNextTask();
    QString buildFFmpegCommand(const QString &inputFile, const QString &outputFile, const QString &format);
    QString buildFFmpegCommandForRawStream(const QString &inputFile, const QString &outputFile, 
                                          const QString &format, const MediaInfo &info);
    QString findFFmpegExecutable();
    
    QQueue<MuxingTask*> m_taskQueue;
    MuxingTask *m_currentTask;
    
    QStringList m_files;
    QString m_outputFolder;
    QString m_outputFormat;
    bool m_overwrite;
    bool m_processing;
    
    int m_currentIndex;
    int m_totalFiles;
    
    QString m_ffmpegPath;
    QList<MediaInfo> m_mediaInfos;
};

#endif // FILEPROCESSOR_H