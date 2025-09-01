#include "FFmpegSetupDialog.h"
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>

FFmpegSetupDialog::FFmpegSetupDialog(QWidget *parent)
    : QDialog(parent)
    , m_testProcess(nullptr)
    , m_progressTimer(new QTimer(this))
    , m_ffmpegFound(false)
    , m_ffprobeFound(false)
{
    setWindowTitle("FFmpeg Setup - Pro Muxer");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);
    resize(600, 500);
    
    setupUI();
    
    // Try auto-detection first
    QString errorMsg;
    if (checkFFmpegAvailability(m_ffmpegPath, m_ffprobePath, errorMsg)) {
        updateStatus("✓ FFmpeg and FFprobe found and working correctly!", true);
        m_okBtn->setEnabled(true);
        m_autoDetectRadio->setChecked(true);
    } else {
        updateStatus("⚠ FFmpeg not found or not working properly", false);
        m_downloadRadio->setChecked(true);
    }
    
    connect(m_progressTimer, &QTimer::timeout, this, &FFmpegSetupDialog::onTestFinished);
}

FFmpegSetupDialog::~FFmpegSetupDialog()
{
    if (m_testProcess) {
        m_testProcess->kill();
        m_testProcess->waitForFinished(1000);
    }
}

void FFmpegSetupDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    
    // Title
    m_titleLabel = new QLabel("FFmpeg Setup Required");
    m_titleLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #2563eb; }");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);
    
    // Status
    m_statusLabel = new QLabel();
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("QLabel { padding: 10px; border: 1px solid #d1d5db; border-radius: 5px; background: #f9fafb; }");
    m_mainLayout->addWidget(m_statusLabel);
    
    // Instructions
    m_instructionText = new QTextEdit();
    m_instructionText->setMaximumHeight(120);
    m_instructionText->setReadOnly(true);
    m_instructionText->setHtml(
        "<p><b>Pro Muxer requires FFmpeg to function.</b> FFmpeg is a free, open-source multimedia framework.</p>"
        "<p>Choose one of the options below to set up FFmpeg:</p>"
    );
    m_mainLayout->addWidget(m_instructionText);
    
    // Setup options
    m_setupGroup = new QGroupBox("Setup Options");
    QVBoxLayout *setupLayout = new QVBoxLayout(m_setupGroup);
    
    // Auto-detect option
    m_autoDetectRadio = new QRadioButton("Auto-detect FFmpeg (if already installed)");
    setupLayout->addWidget(m_autoDetectRadio);
    
    // Manual path option
    m_manualPathRadio = new QRadioButton("Specify FFmpeg location manually");
    setupLayout->addWidget(m_manualPathRadio);
    
    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit();
    m_pathEdit->setPlaceholderText("Path to folder containing ffmpeg.exe and ffprobe.exe");
    m_pathEdit->setEnabled(false);
    m_browseBtn = new QPushButton("Browse");
    m_browseBtn->setEnabled(false);
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(m_browseBtn);
    setupLayout->addLayout(pathLayout);
    
    // Download option
    m_downloadRadio = new QRadioButton("Download FFmpeg (recommended)");
    setupLayout->addWidget(m_downloadRadio);
    
    m_downloadBtn = new QPushButton("Download FFmpeg");
    m_downloadBtn->setEnabled(false);
    m_downloadBtn->setStyleSheet("QPushButton { background-color: #10b981; color: white; padding: 8px; }");
    setupLayout->addWidget(m_downloadBtn);
    
    m_mainLayout->addWidget(m_setupGroup);
    
    // Progress section
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressLabel = new QLabel();
    m_progressLabel->setVisible(false);
    m_mainLayout->addWidget(m_progressLabel);
    m_mainLayout->addWidget(m_progressBar);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_testBtn = new QPushButton("Test Configuration");
    m_retryBtn = new QPushButton("Retry Detection");
    m_okBtn = new QPushButton("OK");
    m_skipBtn = new QPushButton("Skip (Not Recommended)");
    
    m_okBtn->setEnabled(false);
    m_okBtn->setStyleSheet("QPushButton:enabled { background-color: #2563eb; color: white; }");
    
    buttonLayout->addWidget(m_testBtn);
    buttonLayout->addWidget(m_retryBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_skipBtn);
    buttonLayout->addWidget(m_okBtn);
    
    m_mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_autoDetectRadio, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) retryDetection();
    });
    
    connect(m_manualPathRadio, &QRadioButton::toggled, this, [this](bool checked) {
        m_pathEdit->setEnabled(checked);
        m_browseBtn->setEnabled(checked);
        if (checked) m_pathEdit->setFocus();
    });
    
    connect(m_downloadRadio, &QRadioButton::toggled, this, [this](bool checked) {
        m_downloadBtn->setEnabled(checked);
    });
    
    connect(m_browseBtn, &QPushButton::clicked, this, &FFmpegSetupDialog::browseFFmpegFolder);
    connect(m_downloadBtn, &QPushButton::clicked, this, &FFmpegSetupDialog::downloadFFmpeg);
    connect(m_testBtn, &QPushButton::clicked, this, &FFmpegSetupDialog::testFFmpegPath);
    connect(m_retryBtn, &QPushButton::clicked, this, &FFmpegSetupDialog::retryDetection);
    connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_skipBtn, &QPushButton::clicked, this, &QDialog::reject);
}

bool FFmpegSetupDialog::checkFFmpegAvailability(QString &ffmpegPath, QString &ffprobePath, QString &errorMessage)
{
    QProcess ffmpegProcess, ffprobeProcess;
    
#ifdef Q_OS_WIN
    QString ffmpegProgram = "ffmpeg.exe";
    QString ffprobeProgram = "ffprobe.exe";
#else
    QString ffmpegProgram = "ffmpeg";
    QString ffprobeProgram = "ffprobe";
#endif
    
    // Test FFmpeg in PATH
    ffmpegProcess.start(ffmpegProgram, QStringList() << "-version");
    if (!ffmpegProcess.waitForStarted(3000) || !ffmpegProcess.waitForFinished(3000) || ffmpegProcess.exitCode() != 0) {
        errorMessage = "FFmpeg not found in PATH";
        return false;
    }
    
    // Test FFprobe in PATH
    ffprobeProcess.start(ffprobeProgram, QStringList() << "-version");
    if (!ffprobeProcess.waitForStarted(3000) || !ffprobeProcess.waitForFinished(3000) || ffprobeProcess.exitCode() != 0) {
        errorMessage = "FFprobe not found in PATH";
        return false;
    }
    
    // Both found successfully
    ffmpegPath = ffmpegProgram;
    ffprobePath = ffprobeProgram;
    return true;
}

bool FFmpegSetupDialog::showSetupDialogIfNeeded(QWidget *parent)
{
    QString ffmpegPath, ffprobePath, errorMessage;
    if (checkFFmpegAvailability(ffmpegPath, ffprobePath, errorMessage)) {
        return true;  // FFmpeg is available
    }
    
    // Show setup dialog
    FFmpegSetupDialog dialog(parent);
    int result = dialog.exec();
    
    if (result == QDialog::Accepted) {
        // Save paths to settings for future use
        QSettings settings;
        settings.setValue("ffmpeg/ffmpeg_path", dialog.m_ffmpegPath);
        settings.setValue("ffmpeg/ffprobe_path", dialog.m_ffprobePath);
        return true;
    }
    
    return false;  // User skipped or canceled
}

void FFmpegSetupDialog::browseFFmpegFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Select FFmpeg Folder");
    if (!folder.isEmpty()) {
        m_pathEdit->setText(folder);
        
        // Try to find ffmpeg in the selected folder
        QString ffmpegPath = findFFmpegInFolder(folder);
        if (!ffmpegPath.isEmpty()) {
            updateStatus("Found FFmpeg in selected folder", true);
            m_okBtn->setEnabled(true);
            m_ffmpegPath = ffmpegPath;
            
            // Look for ffprobe too
            QFileInfo ffmpegInfo(ffmpegPath);
#ifdef Q_OS_WIN
            QString ffprobePath = ffmpegInfo.absoluteDir().absoluteFilePath("ffprobe.exe");
#else
            QString ffprobePath = ffmpegInfo.absoluteDir().absoluteFilePath("ffprobe");
#endif
            if (QFile::exists(ffprobePath)) {
                m_ffprobePath = ffprobePath;
                updateStatus("✓ Found both FFmpeg and FFprobe in selected folder", true);
            }
        } else {
            updateStatus("FFmpeg not found in selected folder", false);
        }
    }
}

void FFmpegSetupDialog::browseFFmpegExecutable()
{
#ifdef Q_OS_WIN
    QString file = QFileDialog::getOpenFileName(this, "Select ffmpeg.exe", "", "Executable Files (*.exe)");
#else
    QString file = QFileDialog::getOpenFileName(this, "Select ffmpeg", "", "All Files (*)");
#endif
    
    if (!file.isEmpty() && testExecutable(file, "ffmpeg")) {
        m_ffmpegPath = file;
        updateStatus("FFmpeg executable selected", true);
        
        // Look for ffprobe in the same directory
        QFileInfo ffmpegInfo(file);
#ifdef Q_OS_WIN
        QString ffprobePath = ffmpegInfo.absoluteDir().absoluteFilePath("ffprobe.exe");
#else
        QString ffprobePath = ffmpegInfo.absoluteDir().absoluteFilePath("ffprobe");
#endif
        
        if (QFile::exists(ffprobePath) && testExecutable(ffprobePath, "ffprobe")) {
            m_ffprobePath = ffprobePath;
            updateStatus("✓ Found both FFmpeg and FFprobe", true);
            m_okBtn->setEnabled(true);
        } else {
            updateStatus("⚠ FFmpeg found but FFprobe is missing", false);
        }
    }
}

void FFmpegSetupDialog::downloadFFmpeg()
{
    QString message = "This will open the official FFmpeg download page in your web browser.\n\n"
                     "Please download the latest version for Windows and extract it to a folder "
                     "(e.g., C:/ffmpeg/), then use the 'Specify FFmpeg location manually' option.";
    
    QMessageBox::information(this, "Download FFmpeg", message);
    
    QDesktopServices::openUrl(QUrl("https://ffmpeg.org/download.html#build-windows"));
}

void FFmpegSetupDialog::testFFmpegPath()
{
    if (m_manualPathRadio->isChecked()) {
        QString folder = m_pathEdit->text();
        if (folder.isEmpty()) {
            updateStatus("Please enter a folder path", false);
            return;
        }
        
        QString ffmpegPath = findFFmpegInFolder(folder);
        if (ffmpegPath.isEmpty()) {
            updateStatus("FFmpeg not found in specified folder", false);
            return;
        }
        
        if (testExecutable(ffmpegPath, "ffmpeg")) {
            m_ffmpegPath = ffmpegPath;
            updateStatus("✓ FFmpeg test successful", true);
            m_okBtn->setEnabled(true);
        }
    } else {
        retryDetection();
    }
}

void FFmpegSetupDialog::onTestFinished()
{
    m_progressTimer->stop();
    m_progressBar->setVisible(false);
    m_progressLabel->setVisible(false);
}

void FFmpegSetupDialog::retryDetection()
{
    m_progressBar->setVisible(true);
    m_progressLabel->setVisible(true);
    m_progressLabel->setText("Detecting FFmpeg...");
    m_progressBar->setRange(0, 0);  // Indeterminate progress
    
    QString errorMsg;
    if (checkFFmpegAvailability(m_ffmpegPath, m_ffprobePath, errorMsg)) {
        updateStatus("✓ FFmpeg and FFprobe detected successfully!", true);
        m_okBtn->setEnabled(true);
    } else {
        updateStatus("⚠ " + errorMsg, false);
        m_okBtn->setEnabled(false);
    }
    
    m_progressTimer->start(1000);  // Hide progress after 1 second
}

void FFmpegSetupDialog::updateStatus(const QString &message, bool success)
{
    m_statusLabel->setText(message);
    
    if (success) {
        m_statusLabel->setStyleSheet("QLabel { padding: 10px; border: 1px solid #10b981; border-radius: 5px; "
                                    "background: #dcfce7; color: #166534; }");
    } else {
        m_statusLabel->setStyleSheet("QLabel { padding: 10px; border: 1px solid #f59e0b; border-radius: 5px; "
                                    "background: #fef3c7; color: #92400e; }");
    }
}

bool FFmpegSetupDialog::testExecutable(const QString &path, const QString &expectedName)
{
    if (!QFile::exists(path)) {
        return false;
    }
    
    QProcess process;
    process.start(path, QStringList() << "-version");
    if (!process.waitForStarted(3000) || !process.waitForFinished(3000)) {
        return false;
    }
    
    return process.exitCode() == 0;
}

QString FFmpegSetupDialog::findFFmpegInFolder(const QString &folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        return QString();
    }
    
#ifdef Q_OS_WIN
    QString ffmpegName = "ffmpeg.exe";
#else
    QString ffmpegName = "ffmpeg";
#endif
    
    QString directPath = dir.absoluteFilePath(ffmpegName);
    if (QFile::exists(directPath) && testExecutable(directPath, "ffmpeg")) {
        return directPath;
    }
    
    QString binPath = dir.absoluteFilePath("bin/" + ffmpegName);
    if (QFile::exists(binPath) && testExecutable(binPath, "ffmpeg")) {
        return binPath;
    }
    
    return QString();
}