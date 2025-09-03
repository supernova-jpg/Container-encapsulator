#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <QQueue>
#include <QVector>
#include <QMap>

class MuxingTask;
struct MediaInfo;

class FileProcessor : public QObject
{
    Q_OBJECT

public:
    explicit FileProcessor(QObject *parent = nullptr);
    ~FileProcessor();

    void processFiles(const QStringList &files, const QString &outputFolder,
                      const QString &format, const QVector<MediaInfo> &mediaInfos,
                      bool overwrite = false, const QString &processingMode = "muxing");
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

    QString detectVideoFormatFromFileName(const QString &fileName);
    QString parsePixelFormat(const MediaInfo &mediaInfo) const;
    QString generateOutputFilePath(const QString &inputFile, const MediaInfo &mediaInfo);

    QStringList buildFFmpegCommand(const QString &inputFile, const QString &outputFile,
                                   const QString &format, const MediaInfo &mediaInfo);
    QStringList buildBinToYuvCommand(const QString &inputFile, const QString &outputFile,
                                     const MediaInfo &mediaInfo);


    QString findFFmpegExecutable();
    void parseAndLogFFmpegVersion(const QString &versionOutput);


    QQueue<MuxingTask*> m_taskQueue;
    MuxingTask *m_currentTask;

    QStringList m_files;
    QString m_outputFolder;
    QString m_outputFormat;
    QVector<MediaInfo> m_mediaInfos;
    bool m_overwrite;
    bool m_processing;
    QString m_processingMode;

    int m_currentIndex;
    int m_totalFiles;

    QString m_ffmpegPath;
};

#endif // FILEPROCESSOR_H
