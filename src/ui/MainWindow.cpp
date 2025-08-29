#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "../core/FileProcessor.h"
#include "../core/MediaAnalyzer.h"
#include <QApplication>
#include <QDir>
#include <QMimeData>
#include <QStandardPaths>
#include <QHeaderView>
#include <QComboBox>
#include <QDateTime>
#include <QItemDelegate>

// Table column definitions
enum TableColumn {
    COL_FILENAME = 0,
    COL_STATUS = 1,
    COL_VIDEO_CODEC = 2,
    COL_RESOLUTION = 3,
    COL_FRAME_RATE = 4,
    COL_AUDIO_CODEC = 5,
    COL_DURATION = 6,
    COL_FILE_SIZE = 7,
    COL_OUTPUT_NAME = 8
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_processor(nullptr)
    , m_analyzer(nullptr)
    , m_processing(false)
{
    ui->setupUi(this);
    
    // Set initial output folder
    ui->outputFolderEdit->setText(
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/ProMuxer_Output"
    );
    
    // Configure file table
    ui->fileTable->horizontalHeader()->setStretchLastSection(true);
    ui->fileTable->setColumnWidth(COL_FILENAME, 200);
    ui->fileTable->setColumnWidth(COL_STATUS, 100);
    ui->fileTable->setColumnWidth(COL_VIDEO_CODEC, 100);
    ui->fileTable->setColumnWidth(COL_RESOLUTION, 100);
    ui->fileTable->setColumnWidth(COL_FRAME_RATE, 80);
    ui->fileTable->setColumnWidth(COL_AUDIO_CODEC, 120);
    ui->fileTable->setColumnWidth(COL_DURATION, 80);
    ui->fileTable->setColumnWidth(COL_FILE_SIZE, 80);
    
    setupConnections();
    
    // Initialize processors
    m_processor = new FileProcessor(this);
    m_analyzer = new MediaAnalyzer(this);
    
    connect(m_processor, &FileProcessor::progress, this, &MainWindow::onTaskProgress);
    connect(m_processor, &FileProcessor::finished, this, &MainWindow::onTaskFinished);
    connect(m_processor, &FileProcessor::logMessage, this, [this](const QString &msg) {
        onLogMessage(msg, LogLevel::Info);
    });
    
    connect(m_analyzer, &MediaAnalyzer::analysisFinished, this, &MainWindow::onMediaAnalysisFinished);
    connect(m_analyzer, &MediaAnalyzer::analysisError, this, &MainWindow::onMediaAnalysisError);
    
    logMessage("Pro Muxer initialized successfully", LogLevel::Info);
}

MainWindow::~MainWindow()
{
    if (m_processor && m_processing) {
        m_processor->stop();
    }
    delete ui;
}

void MainWindow::setupConnections()
{
    // File operations
    connect(ui->addFilesBtn, &QPushButton::clicked, this, &MainWindow::addFiles);
    connect(ui->addFolderBtn, &QPushButton::clicked, this, &MainWindow::addFolder);
    connect(ui->removeBtn, &QPushButton::clicked, this, &MainWindow::removeSelected);
    connect(ui->clearBtn, &QPushButton::clicked, this, &MainWindow::clearAll);
    connect(ui->analyzeBtn, &QPushButton::clicked, this, &MainWindow::analyzeFiles);
    
    // Settings
    connect(ui->browseBtn, &QPushButton::clicked, this, &MainWindow::browseOutputFolder);
    connect(ui->formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onFormatChanged);
    connect(ui->compatibilityCheck, &QCheckBox::toggled, this, &MainWindow::onCompatibilityToggled);
    
    // Processing
    connect(ui->startBtn, &QPushButton::clicked, this, &MainWindow::startProcessing);
    connect(ui->stopBtn, &QPushButton::clicked, this, &MainWindow::stopProcessing);
    
    // Logging
    connect(ui->clearLogBtn, &QPushButton::clicked, this, &MainWindow::clearLog);
    connect(ui->infoCheck, &QCheckBox::toggled, this, &MainWindow::onLogFilterChanged);
    connect(ui->warningCheck, &QCheckBox::toggled, this, &MainWindow::onLogFilterChanged);
    connect(ui->errorCheck, &QCheckBox::toggled, this, &MainWindow::onLogFilterChanged);
    
    // Table
    connect(ui->fileTable, &QTableWidget::cellChanged, this, &MainWindow::onTableCellChanged);
    connect(ui->fileTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableItemDoubleClicked);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QStringList files;
        foreach (const QUrl &url, mimeData->urls()) {
            QString filePath = url.toLocalFile();
            if (QFileInfo(filePath).isFile() && isVideoFile(filePath)) {
                files << filePath;
            } else if (QFileInfo(filePath).isDir()) {
                QDir dir(filePath);
                QStringList filters;
                filters << "*.mp4" << "*.mkv" << "*.avi" << "*.mov" << "*.wmv" 
                       << "*.flv" << "*.webm" << "*.m4v" << "*.3gp" << "*.ts"
                       << "*.h264" << "*.h265" << "*.bin" << "*.264" << "*.265";
                QFileInfoList fileInfos = dir.entryInfoList(filters, QDir::Files);
                foreach (const QFileInfo &fileInfo, fileInfos) {
                    files << fileInfo.absoluteFilePath();
                }
            }
        }
        addFilesToTable(files);
    }
}

void MainWindow::addFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Video Files",
        QString(),
        "Video Files (*.mp4 *.mkv *.avi *.mov *.wmv *.flv *.webm *.m4v *.3gp *.ts *.h264 *.h265 *.bin *.264 *.265);;All Files (*)"
    );
    
    addFilesToTable(files);
}

void MainWindow::addFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Select Folder");
    if (!folder.isEmpty()) {
        QDir dir(folder);
        QStringList filters;
        filters << "*.mp4" << "*.mkv" << "*.avi" << "*.mov" << "*.wmv" 
               << "*.flv" << "*.webm" << "*.m4v" << "*.3gp" << "*.ts"
               << "*.h264" << "*.h265" << "*.bin" << "*.264" << "*.265";
        
        QFileInfoList fileInfos = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        QStringList files;
        foreach (const QFileInfo &fileInfo, fileInfos) {
            files << fileInfo.absoluteFilePath();
        }
        addFilesToTable(files);
    }
}

void MainWindow::removeSelected()
{
    int row = ui->fileTable->currentRow();
    if (row >= 0) {
        m_files.removeAt(row);
        m_mediaInfos.removeAt(row);
        ui->fileTable->removeRow(row);
        logMessage(QString("Removed file from list"), LogLevel::Info);
    }
}

void MainWindow::clearAll()
{
    m_files.clear();
    m_mediaInfos.clear();
    ui->fileTable->setRowCount(0);
    logMessage("Cleared all files from list", LogLevel::Info);
}

void MainWindow::analyzeFiles()
{
    if (m_files.isEmpty()) {
        logMessage("No files to analyze", LogLevel::Warning);
        return;
    }
    
    logMessage("Starting media analysis...", LogLevel::Info);
    ui->analyzeBtn->setEnabled(false);
    
    for (int i = 0; i < m_files.size(); ++i) {
        m_analyzer->analyzeFile(i, m_files[i]);
        updateTableRowStatus(i, "Analyzing...");
    }
}

void MainWindow::browseOutputFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Select Output Folder");
    if (!folder.isEmpty()) {
        ui->outputFolderEdit->setText(folder);
        updateFileTable(); // Update output names
    }
}

void MainWindow::onFormatChanged()
{
    updateFileTable(); // Update output names and compatibility warnings
}

void MainWindow::onCompatibilityToggled()
{
    if (ui->compatibilityCheck->isChecked()) {
        logMessage("Smart compatibility mode enabled", LogLevel::Info);
        // TODO: Implement smart format suggestions
    }
}

void MainWindow::startProcessing()
{
    if (m_files.isEmpty()) {
        logMessage("No files to process!", LogLevel::Warning);
        return;
    }
    
    QString outputFolder = ui->outputFolderEdit->text();
    if (outputFolder.isEmpty()) {
        logMessage("Please select an output folder!", LogLevel::Error);
        return;
    }
    
    QDir dir;
    if (!dir.exists(outputFolder)) {
        dir.mkpath(outputFolder);
        logMessage(QString("Created output folder: %1").arg(outputFolder), LogLevel::Info);
    }
    
    m_processing = true;
    ui->startBtn->setEnabled(false);
    ui->stopBtn->setEnabled(true);
    ui->progressBar->setValue(0);
    ui->statusLabel->setText("Processing...");
    
    // Determine conflict handling
    bool overwrite = (ui->conflictCombo->currentText() == "Overwrite");
    
    logMessage("Starting batch processing...", LogLevel::Info);
    m_processor->processFiles(m_files, outputFolder, getOutputFormat(), overwrite);
}

void MainWindow::stopProcessing()
{
    if (m_processor) {
        m_processor->stop();
        logMessage("Processing stopped by user", LogLevel::Warning);
    }
    
    m_processing = false;
    ui->startBtn->setEnabled(true);
    ui->stopBtn->setEnabled(false);
    ui->statusLabel->setText("Stopped");
}

void MainWindow::onTaskProgress(int current, int total, const QString &currentFile)
{
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(current);
    ui->statusLabel->setText(QString("Processing: %1 (%2/%3)")
                           .arg(QFileInfo(currentFile).fileName())
                           .arg(current + 1)
                           .arg(total));
    
    // Update table status
    for (int i = 0; i < ui->fileTable->rowCount(); ++i) {
        if (ui->fileTable->item(i, COL_FILENAME)->data(Qt::UserRole).toString() == currentFile) {
            updateTableRowStatus(i, "Processing");
            break;
        }
    }
}

void MainWindow::onTaskFinished()
{
    m_processing = false;
    ui->startBtn->setEnabled(true);
    ui->stopBtn->setEnabled(false);
    ui->statusLabel->setText("Finished");
    ui->progressBar->setValue(ui->progressBar->maximum());
    
    logMessage("All files processed successfully!", LogLevel::Info);
}

void MainWindow::onLogMessage(const QString &message, LogLevel level)
{
    logMessage(message, level);
}

void MainWindow::clearLog()
{
    ui->logEdit->clear();
    logMessage("Log cleared", LogLevel::Info);
}

void MainWindow::onLogFilterChanged()
{
    m_showInfo = ui->infoCheck->isChecked();
    m_showWarning = ui->warningCheck->isChecked();
    m_showError = ui->errorCheck->isChecked();
}

void MainWindow::onMediaAnalysisFinished(int index, const MediaInfo &info)
{
    if (index >= 0 && index < m_mediaInfos.size()) {
        m_mediaInfos[index] = info;
        updateTableRowStatus(index, info.analyzed ? "Ready" : "Analysis Failed");
        
        // Update table cells with analyzed information
        if (index < ui->fileTable->rowCount()) {
            ui->fileTable->setItem(index, COL_VIDEO_CODEC, new QTableWidgetItem(info.videoCodec));
            ui->fileTable->setItem(index, COL_RESOLUTION, new QTableWidgetItem(info.resolution));
            ui->fileTable->setItem(index, COL_FRAME_RATE, new QTableWidgetItem(info.frameRate));
            ui->fileTable->setItem(index, COL_AUDIO_CODEC, new QTableWidgetItem(info.audioCodec));
            ui->fileTable->setItem(index, COL_DURATION, new QTableWidgetItem(info.duration));
            ui->fileTable->setItem(index, COL_FILE_SIZE, new QTableWidgetItem(info.fileSize));
            
            // Add combo box for raw streams
            if (info.isRawStream) {
                QComboBox *codecCombo = new QComboBox();
                codecCombo->addItems({"H.264", "H.265", "MPEG-2", "MPEG-4", "VP9", "AV1"});
                codecCombo->setCurrentText(info.videoCodec);
                ui->fileTable->setCellWidget(index, COL_VIDEO_CODEC, codecCombo);
            }
        }
    }
    
    // Check if all analysis is complete
    static int completedAnalysis = 0;
    completedAnalysis++;
    
    if (completedAnalysis >= m_files.size()) {
        ui->analyzeBtn->setEnabled(true);
        logMessage("Media analysis completed", LogLevel::Info);
        completedAnalysis = 0;
    }
}

void MainWindow::onMediaAnalysisError(int index, const QString &error)
{
    updateTableRowStatus(index, "Analysis Failed");
    logMessage(QString("Analysis failed for file %1: %2").arg(index + 1).arg(error), LogLevel::Error);
}

void MainWindow::onTableCellChanged(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    // Handle table cell changes if needed
}

void MainWindow::onTableItemDoubleClicked(int row, int column)
{
    if (column == COL_OUTPUT_NAME) {
        // Allow editing output name
        ui->fileTable->editItem(ui->fileTable->item(row, column));
    }
}

void MainWindow::addFilesToTable(const QStringList &files)
{
    foreach (const QString &file, files) {
        if (!m_files.contains(file)) {
            m_files << file;
            m_mediaInfos << MediaInfo(); // Add empty media info
            
            int row = ui->fileTable->rowCount();
            ui->fileTable->insertRow(row);
            
            QFileInfo fileInfo(file);
            QTableWidgetItem *nameItem = new QTableWidgetItem(fileInfo.fileName());
            nameItem->setData(Qt::UserRole, file); // Store full path
            ui->fileTable->setItem(row, COL_FILENAME, nameItem);
            
            ui->fileTable->setItem(row, COL_STATUS, new QTableWidgetItem("Ready"));
            ui->fileTable->setItem(row, COL_VIDEO_CODEC, new QTableWidgetItem("Unknown"));
            ui->fileTable->setItem(row, COL_RESOLUTION, new QTableWidgetItem("Unknown"));
            ui->fileTable->setItem(row, COL_FRAME_RATE, new QTableWidgetItem("Unknown"));
            ui->fileTable->setItem(row, COL_AUDIO_CODEC, new QTableWidgetItem("Unknown"));
            ui->fileTable->setItem(row, COL_DURATION, new QTableWidgetItem("Unknown"));
            ui->fileTable->setItem(row, COL_FILE_SIZE, new QTableWidgetItem(formatFileSize(fileInfo.size())));
            
            QString outputName = getOutputFileName(file);
            ui->fileTable->setItem(row, COL_OUTPUT_NAME, new QTableWidgetItem(outputName));
        }
    }
    
    if (!files.isEmpty()) {
        logMessage(QString("Added %1 files to processing list").arg(files.size()), LogLevel::Info);
    }
}

bool MainWindow::isVideoFile(const QString &filePath)
{
    QStringList videoExtensions;
    videoExtensions << "mp4" << "mkv" << "avi" << "mov" << "wmv" << "flv" 
                   << "webm" << "m4v" << "3gp" << "ts" << "h264" << "h265" 
                   << "bin" << "264" << "265";
    
    QString extension = QFileInfo(filePath).suffix().toLower();
    return videoExtensions.contains(extension);
}

QString MainWindow::getOutputFormat() const
{
    return ui->formatCombo->currentText();
}

QString MainWindow::getOutputFileName(const QString &inputFile) const
{
    QFileInfo fileInfo(inputFile);
    QString prefix = ui->prefixEdit->text();
    QString suffix = ui->suffixEdit->text();
    QString format = getOutputFormat();
    
    return prefix + fileInfo.completeBaseName() + suffix + "." + format;
}

QString MainWindow::formatFileSize(qint64 size)
{
    if (size < 1024) {
        return QString::number(size) + " B";
    } else if (size < 1024 * 1024) {
        return QString::number(size / 1024.0, 'f', 1) + " KB";
    } else if (size < 1024 * 1024 * 1024) {
        return QString::number(size / (1024.0 * 1024.0), 'f', 1) + " MB";
    } else {
        return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }
}

void MainWindow::logMessage(const QString &message, LogLevel level)
{
    if (!shouldShowLogLevel(level)) {
        return;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString levelStr;
    QString color;
    
    switch (level) {
    case LogLevel::Info:
        levelStr = "[INFO]";
        color = "black";
        break;
    case LogLevel::Warning:
        levelStr = "[WARN]";
        color = "orange";
        break;
    case LogLevel::Error:
        levelStr = "[ERROR]";
        color = "red";
        break;
    }
    
    QString formattedMessage = QString("<span style='color: %1'>%2 %3 %4</span>")
                              .arg(color)
                              .arg(timestamp)
                              .arg(levelStr)
                              .arg(message);
    
    ui->logEdit->append(formattedMessage);
}

bool MainWindow::shouldShowLogLevel(LogLevel level)
{
    switch (level) {
    case LogLevel::Info: return m_showInfo;
    case LogLevel::Warning: return m_showWarning;
    case LogLevel::Error: return m_showError;
    }
    return true;
}

void MainWindow::updateTableRowStatus(int row, const QString &status)
{
    if (row >= 0 && row < ui->fileTable->rowCount()) {
        ui->fileTable->setItem(row, COL_STATUS, new QTableWidgetItem(status));
    }
}

void MainWindow::showCompatibilityWarning(const QString &codec, const QString &container)
{
    logMessage(QString("Compatibility warning: %1 may not be compatible with %2 container")
              .arg(codec).arg(container), LogLevel::Warning);
}

void MainWindow::updateFileTable()
{
    // Update output names based on current settings
    for (int i = 0; i < ui->fileTable->rowCount(); ++i) {
        if (i < m_files.size()) {
            QString outputName = getOutputFileName(m_files[i]);
            ui->fileTable->setItem(i, COL_OUTPUT_NAME, new QTableWidgetItem(outputName));
        }
    }
}

// CodecComboDelegate implementation
CodecComboDelegate::CodecComboDelegate(QObject *parent)
    : QObject(parent)
{
}

QComboBox* CodecComboDelegate::createComboBox(QWidget *parent)
{
    QComboBox *combo = new QComboBox(parent);
    combo->addItems(getVideoCodecs());
    return combo;
}

QStringList CodecComboDelegate::getVideoCodecs()
{
    return {"H.264", "H.265", "MPEG-2", "MPEG-4", "VP9", "AV1", "ProRes", "DNxHD"};
}

QStringList CodecComboDelegate::getAudioCodecs()
{
    return {"AAC", "MP3", "AC3", "DTS", "PCM", "Vorbis", "Opus"};
}