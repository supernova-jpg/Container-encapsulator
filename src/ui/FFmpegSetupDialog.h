#ifndef FFMPEGSETUPDIALOG_H
#define FFMPEGSETUPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QLineEdit>
#include <QFileDialog>
#include <QProcess>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>
#include <QGroupBox>
#include <QRadioButton>

class FFmpegSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FFmpegSetupDialog(QWidget *parent = nullptr);
    ~FFmpegSetupDialog();

    static bool checkFFmpegAvailability(QString &ffmpegPath, QString &ffprobePath, QString &errorMessage);
    static bool showSetupDialogIfNeeded(QWidget *parent = nullptr);

private slots:
    void browseFFmpegFolder();
    void browseFFmpegExecutable();
    void downloadFFmpeg();
    void testFFmpegPath();
    void onTestFinished();
    void retryDetection();

private:
    void setupUI();
    void updateStatus(const QString &message, bool success = false);
    bool testExecutable(const QString &path, const QString &expectedName);
    QString findFFmpegInFolder(const QString &folderPath);
    
    // UI components
    QVBoxLayout *m_mainLayout;
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QTextEdit *m_instructionText;
    QGroupBox *m_setupGroup;
    QRadioButton *m_autoDetectRadio;
    QRadioButton *m_manualPathRadio;
    QRadioButton *m_downloadRadio;
    
    QLineEdit *m_pathEdit;
    QPushButton *m_browseBtn;
    QPushButton *m_testBtn;
    QPushButton *m_downloadBtn;
    QPushButton *m_retryBtn;
    QPushButton *m_okBtn;
    QPushButton *m_skipBtn;
    
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    
    // Data
    QString m_ffmpegPath;
    QString m_ffprobePath;
    bool m_ffmpegFound;
    bool m_ffprobeFound;
    QProcess *m_testProcess;
    QTimer *m_progressTimer;
};

#endif // FFMPEGSETUPDIALOG_H