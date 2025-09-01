#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "FFmpegSetupDialog.h"
#include "../core/FileProcessor.h"
#include "../core/MediaAnalyzer.h"
#include <QApplication>
#include <QDir>
#include <QMimeData>
#include <QStandardPaths>
#include <QHeaderView>
#include <QComboBox>
#include <QDateTime>
#include <QApplication>
#include <QItemDelegate>
#include <QTimer>
#include <QProcess>
#include <QRegularExpression>
#ifdef Q_OS_WIN
#include <QSettings>
#endif

// Table column definitions
enum TableColumn {
    COL_FILENAME = 0,
    COL_STATUS = 1,
    COL_VIDEO_CODEC = 2,
    COL_RESOLUTION = 3,
    COL_FRAME_RATE = 4,
    COL_BIT_DEPTH = 5,
    COL_COLOR_SPACE = 6,
    COL_DURATION = 7,
    COL_FILE_SIZE = 8,
    COL_OUTPUT_NAME = 9
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
    ui->fileTable->setColumnWidth(COL_RESOLUTION, 120);
    ui->fileTable->setColumnWidth(COL_FRAME_RATE, 90);
    ui->fileTable->setColumnWidth(COL_BIT_DEPTH, 80);
    ui->fileTable->setColumnWidth(COL_COLOR_SPACE, 100);
    ui->fileTable->setColumnWidth(COL_DURATION, 80);
    ui->fileTable->setColumnWidth(COL_FILE_SIZE, 80);
    
    setupConnections();
    
    // Initialize processors
    m_processor = new FileProcessor(this);
    m_analyzer = new MediaAnalyzer(this);
    
    connect(m_processor, &FileProcessor::progress, this, &MainWindow::onTaskProgress);
    connect(m_processor, &FileProcessor::finished, this, &MainWindow::onTaskFinished);
    connect(m_processor, &FileProcessor::logMessage, this, [this](const QString &msg) {
        // Automatically detect log level from message prefix
        LogLevel level = LogLevel::Info;
        if (msg.startsWith("[ERROR]")) {
            level = LogLevel::Error;
        } else if (msg.startsWith("[WARN]") || msg.startsWith("[WARNING]")) {
            level = LogLevel::Warning;
        }
        onLogMessage(msg, level);
    });
    
    connect(m_analyzer, &MediaAnalyzer::analysisFinished, this, &MainWindow::onMediaAnalysisFinished);
    connect(m_analyzer, &MediaAnalyzer::analysisError, this, &MainWindow::onMediaAnalysisError);
    
    logMessage("Pro Muxer initialized successfully", LogLevel::Info);
    
    // Check FFmpeg environment after UI is fully loaded
    QTimer::singleShot(500, this, &MainWindow::checkFFmpegEnvironment);
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
        if (!files.isEmpty()) {
            addFilesToTable(files);
            analyzeFiles(); // Automatically analyze dropped files
        }
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
    
    if (!files.isEmpty()) {
        addFilesToTable(files);
        analyzeFiles(); // Automatically analyze added files
    }
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
        if (!files.isEmpty()) {
            addFilesToTable(files);
            analyzeFiles(); // Automatically analyze added files
        }
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
        logMessage("[WARNING] No files to analyze", LogLevel::Warning);
        return;
    }
    
    logMessage("Starting media analysis...", LogLevel::Info);
    
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
        logMessage("[WARNING] No files to process!", LogLevel::Warning);
        return;
    }
    
    QString outputFolder = ui->outputFolderEdit->text();
    if (outputFolder.isEmpty()) {
        logMessage("[ERROR] Please select an output folder!", LogLevel::Error);
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
    m_processor->processFiles(m_files, outputFolder, getOutputFormat(), m_mediaInfos, overwrite);
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
            ui->fileTable->setItem(index, COL_BIT_DEPTH, new QTableWidgetItem(info.bitDepth));
            ui->fileTable->setItem(index, COL_COLOR_SPACE, new QTableWidgetItem(info.colorSpace));
            ui->fileTable->setItem(index, COL_DURATION, new QTableWidgetItem(info.duration));
            ui->fileTable->setItem(index, COL_FILE_SIZE, new QTableWidgetItem(info.fileSize));
            
            // Add editable combo boxes for key metadata fields
            setupEditableCell(index, COL_VIDEO_CODEC, info.videoCodec, getVideoCodecOptions());
            setupEditableCell(index, COL_RESOLUTION, info.resolution, getResolutionPresets());
            setupEditableCell(index, COL_FRAME_RATE, info.frameRate, getFrameRateOptions());
            setupEditableCell(index, COL_BIT_DEPTH, info.bitDepth, getBitDepthOptions());
            setupEditableCell(index, COL_COLOR_SPACE, info.colorSpace, getColorSpaceOptions());
        }
    }
    
    // Check if all analysis is complete
    static int completedAnalysis = 0;
    completedAnalysis++;
    
    if (completedAnalysis >= m_files.size()) {
        logMessage("Media analysis completed", LogLevel::Info);
        completedAnalysis = 0;
    }
}

void MainWindow::onMediaAnalysisError(int index, const QString &error)
{
    updateTableRowStatus(index, "Smart Defaults Applied");
    logMessage(QString("Analysis failed for file %1: %2. Applying intelligent fallback defaults.").arg(index + 1).arg(error), LogLevel::Warning);
    
    // Create default MediaInfo with enhanced intelligent guesses and enable editing
    if (index >= 0 && index < m_mediaInfos.size() && index < m_files.size()) {
        QString filePath = m_files[index];
        MediaInfo info;
        
        // Make intelligent guesses based on filename and extension
        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();
        QString fileName = fileInfo.baseName().toLower();
        QString fullFileName = fileInfo.fileName().toLower();
        
        // Enhanced codec guessing from extension and filename patterns
        if (extension == "h264" || extension == "264" || fullFileName.contains("avc")) {
            info.videoCodec = "H.264";
            info.bitDepth = "8 bit";
        } else if (extension == "h265" || extension == "hevc" || extension == "265" || fullFileName.contains("hevc")) {
            info.videoCodec = "H.265/HEVC";
            info.bitDepth = fullFileName.contains("hdr") || fullFileName.contains("10bit") ? "10 bit" : "8 bit";
        } else if (fullFileName.contains("av1")) {
            info.videoCodec = "AV1";
            info.bitDepth = "10 bit";
        } else if (fullFileName.contains("vp9")) {
            info.videoCodec = "VP9";
            info.bitDepth = "8 bit";
        } else if (extension == "bin" && fullFileName.contains("prores")) {
            info.videoCodec = "ProRes";
            info.bitDepth = "10 bit";
        } else {
            info.videoCodec = "H.264"; // Most common fallback
            info.bitDepth = "8 bit";
        }
        
        // Enhanced resolution guessing with more patterns
        if (fileName.contains("8k") || fileName.contains("4320") || fileName.contains("7680")) {
            info.resolution = "7680x4320 (8K UHD)";
            info.colorSpace = "Rec. 2020 (HDR)";
        } else if (fileName.contains("4k") || fileName.contains("2160") || fileName.contains("3840") || fileName.contains("uhd")) {
            info.resolution = "3840x2160 (4K UHD)";
            info.colorSpace = fullFileName.contains("hdr") ? "Rec. 2020 (HDR)" : "Rec. 709 (sRGB)";
        } else if (fileName.contains("qhd") || fileName.contains("1440") || fileName.contains("2560")) {
            info.resolution = "2560x1440 (QHD)";
            info.colorSpace = "Rec. 709 (sRGB)";
        } else if (fileName.contains("fhd") || fileName.contains("1080") || fileName.contains("1920")) {
            info.resolution = "1920x1080 (FHD)";
            info.colorSpace = "Rec. 709 (sRGB)";
        } else if (fileName.contains("hd") || fileName.contains("720") || fileName.contains("1280")) {
            info.resolution = "1280x720 (HD)";
            info.colorSpace = "Rec. 709 (sRGB)";
        } else if (fileName.contains("480p") || fileName.contains("854")) {
            info.resolution = "854x480";
            info.colorSpace = "Rec. 601 (SDTV)";
        } else {
            // Default based on file size heuristics
            qint64 fileSize = fileInfo.size();
            if (fileSize > 500 * 1024 * 1024) { // >500MB likely 4K
                info.resolution = "3840x2160 (4K UHD)";
                info.colorSpace = "Rec. 709 (sRGB)";
            } else if (fileSize > 100 * 1024 * 1024) { // >100MB likely FHD
                info.resolution = "1920x1080 (FHD)";
                info.colorSpace = "Rec. 709 (sRGB)";
            } else {
                info.resolution = "1280x720 (HD)";
                info.colorSpace = "Rec. 709 (sRGB)";
            }
        }
        
        // Enhanced frame rate guessing with more patterns
        if (fileName.contains("120fps") || fileName.contains("120p")) {
            info.frameRate = "120.000 fps";
        } else if (fileName.contains("100fps") || fileName.contains("100p")) {
            info.frameRate = "100.000 fps";
        } else if (fileName.contains("60fps") || fileName.contains("60p")) {
            info.frameRate = "60.000 fps";
        } else if (fileName.contains("59.94") || fileName.contains("5994")) {
            info.frameRate = "59.940 fps";
        } else if (fileName.contains("50fps") || fileName.contains("50p")) {
            info.frameRate = "50.000 fps";
        } else if (fileName.contains("48fps") || fileName.contains("48p")) {
            info.frameRate = "48.000 fps";
        } else if (fileName.contains("30fps") || fileName.contains("30p")) {
            info.frameRate = "30.000 fps";
        } else if (fileName.contains("29.97") || fileName.contains("2997")) {
            info.frameRate = "29.970 fps";
        } else if (fileName.contains("25fps") || fileName.contains("25p")) {
            info.frameRate = "25.000 fps";
        } else if (fileName.contains("24fps") || fileName.contains("24p") || fileName.contains("cinema")) {
            info.frameRate = "24.000 fps";
        } else if (fileName.contains("23.976") || fileName.contains("23976")) {
            info.frameRate = "23.976 fps";
        } else {
            // Smart default based on resolution
            if (info.resolution.contains("4K") || info.resolution.contains("8K")) {
                info.frameRate = "24.000 fps"; // 4K/8K often cinema
            } else if (fileName.contains("pal") || fileName.contains("europe")) {
                info.frameRate = "25.000 fps"; // PAL regions
            } else {
                info.frameRate = "30.000 fps"; // NTSC default
            }
        }
        
        // Enhanced bit depth detection
        if (fullFileName.contains("10bit") || fullFileName.contains("10-bit")) {
            info.bitDepth = "10 bit";
        } else if (fullFileName.contains("12bit") || fullFileName.contains("12-bit")) {
            info.bitDepth = "12 bit";
        } else if (fullFileName.contains("16bit") || fullFileName.contains("16-bit")) {
            info.bitDepth = "16 bit";
        } else if (fullFileName.contains("hdr")) {
            info.bitDepth = "10 bit"; // HDR typically 10-bit
        }
        
        // Enhanced color space detection
        if (fullFileName.contains("hdr") || fullFileName.contains("rec2020") || fullFileName.contains("bt2020")) {
            info.colorSpace = "Rec. 2020 (HDR)";
        } else if (fullFileName.contains("p3") || fullFileName.contains("dci-p3")) {
            info.colorSpace = "DCI-P3";
        } else if (fullFileName.contains("rec601") || fullFileName.contains("bt601")) {
            info.colorSpace = "Rec. 601 (SDTV)";
        } else if (fullFileName.contains("adobe") || fullFileName.contains("adobergb")) {
            info.colorSpace = "Adobe RGB";
        }
        
        // Set other default values
        info.duration = "Unknown";
        info.fileSize = formatFileSize(fileInfo.size());
        info.isRawStream = true;
        info.analyzed = false; // Mark as not analyzed to indicate manual editing needed
        
        m_mediaInfos[index] = info;
        
        // Update table with editable cells immediately
        if (index < ui->fileTable->rowCount()) {
            setupEditableCell(index, COL_VIDEO_CODEC, info.videoCodec, getVideoCodecOptions());
            setupEditableCell(index, COL_RESOLUTION, info.resolution, getResolutionPresets());
            setupEditableCell(index, COL_FRAME_RATE, info.frameRate, getFrameRateOptions());
            setupEditableCell(index, COL_BIT_DEPTH, info.bitDepth, getBitDepthOptions());
            setupEditableCell(index, COL_COLOR_SPACE, info.colorSpace, getColorSpaceOptions());
            
            ui->fileTable->setItem(index, COL_DURATION, new QTableWidgetItem(info.duration));
            ui->fileTable->setItem(index, COL_FILE_SIZE, new QTableWidgetItem(info.fileSize));
            
            // Highlight editable cells to indicate manual review is needed
            for (int col = COL_VIDEO_CODEC; col <= COL_COLOR_SPACE; ++col) {
                if (ui->fileTable->cellWidget(index, col)) {
                    ui->fileTable->cellWidget(index, col)->setStyleSheet(
                        "QComboBox { font-size: 12px; padding: 2px; background-color: #fff3cd; border: 2px solid #ffc107; }"
                    );
                }
            }
        }
        
        logMessage(QString("Smart defaults applied for file %1 based on filename patterns. Yellow highlighting indicates manual review recommended.").arg(index + 1), LogLevel::Info);
    }
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
            ui->fileTable->setItem(row, COL_BIT_DEPTH, new QTableWidgetItem("Unknown"));
            ui->fileTable->setItem(row, COL_COLOR_SPACE, new QTableWidgetItem("Unknown"));
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
    
    // Detect Windows theme and adapt colors accordingly
    bool isDarkTheme = false;
#ifdef Q_OS_WIN
    // Check if Windows is in dark mode
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 
                      QSettings::NativeFormat);
    isDarkTheme = (settings.value("AppsUseLightTheme", 1).toInt() == 0);
#endif
    
    // Use palette-aware colors as fallback
    QPalette palette = QApplication::palette();
    QString defaultTextColor = palette.color(QPalette::WindowText).name();
    
    switch (level) {
    case LogLevel::Info:
        levelStr = "[INFO]";
        color = isDarkTheme ? "#FFFFFF" : (defaultTextColor.isEmpty() ? "#000000" : defaultTextColor);
        break;
    case LogLevel::Warning:
        levelStr = "[WARN]";
        color = "#FFA500"; // Orange works in both themes
        break;
    case LogLevel::Error:
        levelStr = "[ERROR]";
        color = "#FF4444"; // Lighter red for better visibility in dark themes
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

void MainWindow::setupEditableCell(int row, int column, const QString &currentValue, const QStringList &options)
{
    QComboBox *combo = new QComboBox();
    combo->setEditable(true);
    combo->addItems(options);
    combo->setCurrentText(currentValue);
    combo->setStyleSheet("QComboBox { font-size: 12px; padding: 2px; }");
    
    // Connect to update MediaInfo when value changes
    connect(combo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), 
            this, [this, row, column](const QString &text) {
        // Update the underlying MediaInfo structure
        if (row < m_mediaInfos.size()) {
            MediaInfo &info = m_mediaInfos[row];
            switch (column) {
                case COL_VIDEO_CODEC: info.videoCodec = text; break;
                case COL_RESOLUTION: info.resolution = text; break;
                case COL_FRAME_RATE: info.frameRate = text; break;
                case COL_BIT_DEPTH: info.bitDepth = text; break;
                case COL_COLOR_SPACE: info.colorSpace = text; break;
            }
            logMessage(QString("Updated %1 for file %2 to: %3")
                      .arg(ui->fileTable->horizontalHeaderItem(column)->text())
                      .arg(row + 1)
                      .arg(text), LogLevel::Info);
        }
    });
    
    ui->fileTable->setCellWidget(row, column, combo);
}

QStringList MainWindow::getVideoCodecOptions()
{
    return {"H.264", "H.265/HEVC", "AV1", "VP9", "VP8", "MPEG-2", "MPEG-4", "ProRes", "DNxHD", "Unknown"};
}

QStringList MainWindow::getResolutionPresets()
{
    return {
        "3840x2160 (4K UHD)",
        "7680x4320 (8K UHD)", 
        "1920x1080 (FHD)",
        "1280x720 (HD)",
        "2560x1440 (QHD)",
        "3440x1440 (UWQHD)",
        "1920x800 (Cinema)",
        "1680x1050",
        "1600x900",
        "1366x768",
        "1280x960",
        "1024x768",
        "854x480",
        "640x480",
        "Custom",
        "Unknown"
    };
}

QStringList MainWindow::getFrameRateOptions()
{
    return {
        "23.976 fps",
        "24.000 fps", 
        "25.000 fps",
        "29.970 fps",
        "30.000 fps",
        "48.000 fps",
        "50.000 fps",
        "59.940 fps", 
        "60.000 fps",
        "100.000 fps",
        "120.000 fps",
        "Variable",
        "Unknown"
    };
}

QStringList MainWindow::getBitDepthOptions()
{
    return {
        "8 bit",
        "10 bit", 
        "12 bit",
        "16 bit",
        "32 bit",
        "Unknown"
    };
}

QStringList MainWindow::getColorSpaceOptions()
{
    return {
        "Rec. 709 (sRGB)",
        "Rec. 2020 (HDR)",
        "Rec. 601 (SDTV)",
        "DCI-P3",
        "Adobe RGB",
        "ProPhoto RGB",
        "SMPTE 170M",
        "BT.470 System M",
        "BT.470 System B/G",
        "Linear",
        "Unknown"
    };
}

void MainWindow::checkFFmpegEnvironment()
{
    QString ffmpegPath, ffprobePath, errorMessage;
    
    if (!FFmpegSetupDialog::checkFFmpegAvailability(ffmpegPath, ffprobePath, errorMessage)) {
        logMessage("FFmpeg environment check failed: " + errorMessage, LogLevel::Warning);
        
        // Show setup dialog
        if (!FFmpegSetupDialog::showSetupDialogIfNeeded(this)) {
            logMessage("User chose to skip FFmpeg setup. Some features may not work correctly.", LogLevel::Warning);
        } else {
            logMessage("FFmpeg environment configured successfully", LogLevel::Info);
        }
    } else {
        logMessage("FFmpeg environment is ready", LogLevel::Info);
        logFFmpegEnvironmentDetails();
    }
}

void MainWindow::logFFmpegEnvironmentDetails()
{
    // Ask core components to emit version logs (constructor already attempted detection)
    // Re-run a lightweight version probe to ensure info is visible in UI logs at startup
    QProcess probe;
#ifdef Q_OS_WIN
    const QString ffmpegProgram = "ffmpeg.exe";
    const QString ffprobeProgram = "ffprobe.exe";
#else
    const QString ffmpegProgram = "ffmpeg";
    const QString ffprobeProgram = "ffprobe";
#endif

    // Try explicit env overrides first
    QString envFFmpeg = qEnvironmentVariable("FFMPEG_PATH");
    QString envFFprobe = qEnvironmentVariable("FFPROBE_PATH");

    auto logVersion = [&](const QString &program, const QString &label) {
        QString exe = program;
        if (!QFileInfo(exe).isExecutable()) {
            // Fall back to PATH name
            exe = label.contains("FFprobe") ? ffprobeProgram : ffmpegProgram;
        }
        probe.start(exe, QStringList() << "--version");
        if (probe.waitForStarted(2000) && probe.waitForFinished(3000) && probe.exitCode() == 0) {
            QString out = QString::fromUtf8(probe.readAllStandardOutput() + probe.readAllStandardError());
            QString firstLine = out.split('\n').value(0);

            // Extract version and year range
            QString version = "Unknown";
            QString years = "Unknown";
            QRegularExpression verRe(label.contains("FFprobe") ?
                                     QRegularExpression("ffprobe version ([\\\\d\\\\.\\\\w-]+)") :
                                     QRegularExpression("ffmpeg version ([\\\\d\\\\.\\\\w-]+)"));
            QRegularExpressionMatch vm = verRe.match(firstLine);
            if (vm.hasMatch()) version = vm.captured(1);

            QRegularExpression yrRe("Copyright \\\\(c\\\\) (\\\\d{4})-(\\\\d{4})");
            QRegularExpressionMatch ym = yrRe.match(firstLine);
            if (ym.hasMatch()) years = QString("%1-%2").arg(ym.captured(1), ym.captured(2));

            logMessage(QString("%1 found in PATH - Version: %2, Release Years: %3")
                           .arg(label, version, years), LogLevel::Info);
        }
    };

    if (!envFFmpeg.isEmpty()) logVersion(envFFmpeg, "FFmpeg"); else logVersion(ffmpegProgram, "FFmpeg");
    if (!envFFprobe.isEmpty()) logVersion(envFFprobe, "FFprobe"); else logVersion(ffprobeProgram, "FFprobe");
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