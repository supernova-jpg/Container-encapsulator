/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainLayout;
    QGroupBox *fileGroup;
    QHBoxLayout *fileButtonLayout;
    QPushButton *addFilesBtn;
    QPushButton *addFolderBtn;
    QPushButton *removeBtn;
    QPushButton *clearBtn;
    QPushButton *applyAllBtn;
    QSpacerItem *fileButtonSpacer;
    QTableWidget *fileTable;
    QGroupBox *settingsGroup;
    QGridLayout *settingsLayout;
    QLabel *outputFolderLabel;
    QLineEdit *outputFolderEdit;
    QPushButton *browseBtn;
    QLabel *formatLabel;
    QComboBox *formatCombo;
    QLabel *namingLabel;
    QHBoxLayout *namingLayout;
    QLineEdit *prefixEdit;
    QLabel *namingMiddleLabel;
    QLineEdit *suffixEdit;
    QComboBox *conflictCombo;
    QCheckBox *compatibilityCheck;
    QWidget *binToYuvGroup;
    QGridLayout *binToYuvLayout;
    QLabel *sequenceLabel;
    QLineEdit *sequenceEdit;
    QLabel *bitDepthYuvLabel;
    QComboBox *bitDepthYuvCombo;
    QLabel *sceneLabel;
    QLineEdit *sceneEdit;
    QLabel *resolutionYuvLabel;
    QComboBox *resolutionYuvCombo;
    QLabel *frameRateYuvLabel;
    QComboBox *frameRateYuvCombo;
    QLabel *colorFormatLabel;
    QComboBox *colorFormatCombo;
    QLabel *previewLabel;
    QGroupBox *processGroup;
    QVBoxLayout *processLayout;
    QHBoxLayout *modeLayout;
    QLabel *modeLabel;
    QRadioButton *muxingModeRadio;
    QRadioButton *binToYuvModeRadio;
    QSpacerItem *modeSpacer;
    QHBoxLayout *processButtonsLayout;
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QGroupBox *logGroup;
    QVBoxLayout *logLayout;
    QHBoxLayout *logControlLayout;
    QCheckBox *infoCheck;
    QCheckBox *warningCheck;
    QCheckBox *errorCheck;
    QSpacerItem *logControlSpacer;
    QPushButton *clearLogBtn;
    QTextEdit *logEdit;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        MainWindow->setAcceptDrops(true);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainLayout = new QVBoxLayout(centralwidget);
        mainLayout->setSpacing(10);
        mainLayout->setObjectName("mainLayout");
        fileGroup = new QGroupBox(centralwidget);
        fileGroup->setObjectName("fileGroup");
        fileButtonLayout = new QHBoxLayout(fileGroup);
        fileButtonLayout->setObjectName("fileButtonLayout");
        addFilesBtn = new QPushButton(fileGroup);
        addFilesBtn->setObjectName("addFilesBtn");

        fileButtonLayout->addWidget(addFilesBtn);

        addFolderBtn = new QPushButton(fileGroup);
        addFolderBtn->setObjectName("addFolderBtn");

        fileButtonLayout->addWidget(addFolderBtn);

        removeBtn = new QPushButton(fileGroup);
        removeBtn->setObjectName("removeBtn");

        fileButtonLayout->addWidget(removeBtn);

        clearBtn = new QPushButton(fileGroup);
        clearBtn->setObjectName("clearBtn");

        fileButtonLayout->addWidget(clearBtn);

        applyAllBtn = new QPushButton(fileGroup);
        applyAllBtn->setObjectName("applyAllBtn");
        applyAllBtn->setEnabled(false);

        fileButtonLayout->addWidget(applyAllBtn);

        fileButtonSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        fileButtonLayout->addItem(fileButtonSpacer);


        mainLayout->addWidget(fileGroup);

        fileTable = new QTableWidget(centralwidget);
        if (fileTable->columnCount() < 10)
            fileTable->setColumnCount(10);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        fileTable->setHorizontalHeaderItem(9, __qtablewidgetitem9);
        fileTable->setObjectName("fileTable");
        fileTable->setAlternatingRowColors(true);
        fileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        fileTable->setSortingEnabled(true);

        mainLayout->addWidget(fileTable);

        settingsGroup = new QGroupBox(centralwidget);
        settingsGroup->setObjectName("settingsGroup");
        settingsGroup->setMaximumHeight(120);
        settingsLayout = new QGridLayout(settingsGroup);
        settingsLayout->setObjectName("settingsLayout");
        outputFolderLabel = new QLabel(settingsGroup);
        outputFolderLabel->setObjectName("outputFolderLabel");

        settingsLayout->addWidget(outputFolderLabel, 0, 0, 1, 1);

        outputFolderEdit = new QLineEdit(settingsGroup);
        outputFolderEdit->setObjectName("outputFolderEdit");

        settingsLayout->addWidget(outputFolderEdit, 0, 1, 1, 1);

        browseBtn = new QPushButton(settingsGroup);
        browseBtn->setObjectName("browseBtn");
        browseBtn->setMaximumWidth(80);

        settingsLayout->addWidget(browseBtn, 0, 2, 1, 1);

        formatLabel = new QLabel(settingsGroup);
        formatLabel->setObjectName("formatLabel");

        settingsLayout->addWidget(formatLabel, 1, 0, 1, 1);

        formatCombo = new QComboBox(settingsGroup);
        formatCombo->addItem(QString());
        formatCombo->addItem(QString());
        formatCombo->addItem(QString());
        formatCombo->addItem(QString());
        formatCombo->addItem(QString());
        formatCombo->setObjectName("formatCombo");

        settingsLayout->addWidget(formatCombo, 1, 1, 1, 1);

        namingLabel = new QLabel(settingsGroup);
        namingLabel->setObjectName("namingLabel");

        settingsLayout->addWidget(namingLabel, 2, 0, 1, 1);

        namingLayout = new QHBoxLayout();
        namingLayout->setObjectName("namingLayout");
        prefixEdit = new QLineEdit(settingsGroup);
        prefixEdit->setObjectName("prefixEdit");

        namingLayout->addWidget(prefixEdit);

        namingMiddleLabel = new QLabel(settingsGroup);
        namingMiddleLabel->setObjectName("namingMiddleLabel");

        namingLayout->addWidget(namingMiddleLabel);

        suffixEdit = new QLineEdit(settingsGroup);
        suffixEdit->setObjectName("suffixEdit");

        namingLayout->addWidget(suffixEdit);


        settingsLayout->addLayout(namingLayout, 2, 1, 1, 1);

        conflictCombo = new QComboBox(settingsGroup);
        conflictCombo->addItem(QString());
        conflictCombo->addItem(QString());
        conflictCombo->addItem(QString());
        conflictCombo->setObjectName("conflictCombo");

        settingsLayout->addWidget(conflictCombo, 1, 2, 1, 1);

        compatibilityCheck = new QCheckBox(settingsGroup);
        compatibilityCheck->setObjectName("compatibilityCheck");

        settingsLayout->addWidget(compatibilityCheck, 2, 2, 1, 1);

        binToYuvGroup = new QWidget(settingsGroup);
        binToYuvGroup->setObjectName("binToYuvGroup");
        binToYuvGroup->setVisible(false);
        binToYuvLayout = new QGridLayout(binToYuvGroup);
        binToYuvLayout->setObjectName("binToYuvLayout");
        binToYuvLayout->setContentsMargins(0, 0, 0, 0);
        sequenceLabel = new QLabel(binToYuvGroup);
        sequenceLabel->setObjectName("sequenceLabel");

        binToYuvLayout->addWidget(sequenceLabel, 0, 0, 1, 1);

        sequenceEdit = new QLineEdit(binToYuvGroup);
        sequenceEdit->setObjectName("sequenceEdit");

        binToYuvLayout->addWidget(sequenceEdit, 0, 1, 1, 1);

        bitDepthYuvLabel = new QLabel(binToYuvGroup);
        bitDepthYuvLabel->setObjectName("bitDepthYuvLabel");

        binToYuvLayout->addWidget(bitDepthYuvLabel, 0, 2, 1, 1);

        bitDepthYuvCombo = new QComboBox(binToYuvGroup);
        bitDepthYuvCombo->addItem(QString());
        bitDepthYuvCombo->addItem(QString());
        bitDepthYuvCombo->addItem(QString());
        bitDepthYuvCombo->setObjectName("bitDepthYuvCombo");

        binToYuvLayout->addWidget(bitDepthYuvCombo, 0, 3, 1, 1);

        sceneLabel = new QLabel(binToYuvGroup);
        sceneLabel->setObjectName("sceneLabel");

        binToYuvLayout->addWidget(sceneLabel, 1, 0, 1, 1);

        sceneEdit = new QLineEdit(binToYuvGroup);
        sceneEdit->setObjectName("sceneEdit");

        binToYuvLayout->addWidget(sceneEdit, 1, 1, 1, 3);

        resolutionYuvLabel = new QLabel(binToYuvGroup);
        resolutionYuvLabel->setObjectName("resolutionYuvLabel");

        binToYuvLayout->addWidget(resolutionYuvLabel, 2, 0, 1, 1);

        resolutionYuvCombo = new QComboBox(binToYuvGroup);
        resolutionYuvCombo->addItem(QString());
        resolutionYuvCombo->addItem(QString());
        resolutionYuvCombo->addItem(QString());
        resolutionYuvCombo->addItem(QString());
        resolutionYuvCombo->setObjectName("resolutionYuvCombo");

        binToYuvLayout->addWidget(resolutionYuvCombo, 2, 1, 1, 1);

        frameRateYuvLabel = new QLabel(binToYuvGroup);
        frameRateYuvLabel->setObjectName("frameRateYuvLabel");

        binToYuvLayout->addWidget(frameRateYuvLabel, 2, 2, 1, 1);

        frameRateYuvCombo = new QComboBox(binToYuvGroup);
        frameRateYuvCombo->addItem(QString());
        frameRateYuvCombo->addItem(QString());
        frameRateYuvCombo->addItem(QString());
        frameRateYuvCombo->addItem(QString());
        frameRateYuvCombo->setObjectName("frameRateYuvCombo");

        binToYuvLayout->addWidget(frameRateYuvCombo, 2, 3, 1, 1);

        colorFormatLabel = new QLabel(binToYuvGroup);
        colorFormatLabel->setObjectName("colorFormatLabel");

        binToYuvLayout->addWidget(colorFormatLabel, 3, 0, 1, 1);

        colorFormatCombo = new QComboBox(binToYuvGroup);
        colorFormatCombo->addItem(QString());
        colorFormatCombo->addItem(QString());
        colorFormatCombo->setObjectName("colorFormatCombo");

        binToYuvLayout->addWidget(colorFormatCombo, 3, 1, 1, 1);

        previewLabel = new QLabel(binToYuvGroup);
        previewLabel->setObjectName("previewLabel");

        binToYuvLayout->addWidget(previewLabel, 3, 2, 1, 2);


        settingsLayout->addWidget(binToYuvGroup, 3, 0, 1, 3);


        mainLayout->addWidget(settingsGroup);

        processGroup = new QGroupBox(centralwidget);
        processGroup->setObjectName("processGroup");
        processGroup->setMaximumHeight(120);
        processLayout = new QVBoxLayout(processGroup);
        processLayout->setObjectName("processLayout");
        modeLayout = new QHBoxLayout();
        modeLayout->setObjectName("modeLayout");
        modeLabel = new QLabel(processGroup);
        modeLabel->setObjectName("modeLabel");

        modeLayout->addWidget(modeLabel);

        muxingModeRadio = new QRadioButton(processGroup);
        muxingModeRadio->setObjectName("muxingModeRadio");
        muxingModeRadio->setChecked(true);

        modeLayout->addWidget(muxingModeRadio);

        binToYuvModeRadio = new QRadioButton(processGroup);
        binToYuvModeRadio->setObjectName("binToYuvModeRadio");

        modeLayout->addWidget(binToYuvModeRadio);

        modeSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        modeLayout->addItem(modeSpacer);


        processLayout->addLayout(modeLayout);

        processButtonsLayout = new QHBoxLayout();
        processButtonsLayout->setObjectName("processButtonsLayout");
        startBtn = new QPushButton(processGroup);
        startBtn->setObjectName("startBtn");

        processButtonsLayout->addWidget(startBtn);

        stopBtn = new QPushButton(processGroup);
        stopBtn->setObjectName("stopBtn");
        stopBtn->setEnabled(false);

        processButtonsLayout->addWidget(stopBtn);

        progressBar = new QProgressBar(processGroup);
        progressBar->setObjectName("progressBar");

        processButtonsLayout->addWidget(progressBar);

        statusLabel = new QLabel(processGroup);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setMinimumWidth(200);

        processButtonsLayout->addWidget(statusLabel);


        processLayout->addLayout(processButtonsLayout);


        mainLayout->addWidget(processGroup);

        logGroup = new QGroupBox(centralwidget);
        logGroup->setObjectName("logGroup");
        logGroup->setMaximumHeight(200);
        logLayout = new QVBoxLayout(logGroup);
        logLayout->setObjectName("logLayout");
        logControlLayout = new QHBoxLayout();
        logControlLayout->setObjectName("logControlLayout");
        infoCheck = new QCheckBox(logGroup);
        infoCheck->setObjectName("infoCheck");
        infoCheck->setChecked(true);

        logControlLayout->addWidget(infoCheck);

        warningCheck = new QCheckBox(logGroup);
        warningCheck->setObjectName("warningCheck");
        warningCheck->setChecked(true);

        logControlLayout->addWidget(warningCheck);

        errorCheck = new QCheckBox(logGroup);
        errorCheck->setObjectName("errorCheck");
        errorCheck->setChecked(true);

        logControlLayout->addWidget(errorCheck);

        logControlSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        logControlLayout->addItem(logControlSpacer);

        clearLogBtn = new QPushButton(logGroup);
        clearLogBtn->setObjectName("clearLogBtn");
        clearLogBtn->setMaximumWidth(80);

        logControlLayout->addWidget(clearLogBtn);


        logLayout->addLayout(logControlLayout);

        logEdit = new QTextEdit(logGroup);
        logEdit->setObjectName("logEdit");
        logEdit->setReadOnly(true);

        logLayout->addWidget(logEdit);


        mainLayout->addWidget(logGroup);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Pro Muxer - Professional Video Stream Muxing Tool", nullptr));
        fileGroup->setTitle(QCoreApplication::translate("MainWindow", "File Operations", nullptr));
        addFilesBtn->setText(QCoreApplication::translate("MainWindow", "Add Files", nullptr));
#if QT_CONFIG(tooltip)
        addFilesBtn->setToolTip(QCoreApplication::translate("MainWindow", "Add individual video files", nullptr));
#endif // QT_CONFIG(tooltip)
        addFolderBtn->setText(QCoreApplication::translate("MainWindow", "Add Folder", nullptr));
#if QT_CONFIG(tooltip)
        addFolderBtn->setToolTip(QCoreApplication::translate("MainWindow", "Add all video files from a folder", nullptr));
#endif // QT_CONFIG(tooltip)
        removeBtn->setText(QCoreApplication::translate("MainWindow", "Remove Selected", nullptr));
#if QT_CONFIG(tooltip)
        removeBtn->setToolTip(QCoreApplication::translate("MainWindow", "Remove selected files from the list", nullptr));
#endif // QT_CONFIG(tooltip)
        clearBtn->setText(QCoreApplication::translate("MainWindow", "Clear All", nullptr));
#if QT_CONFIG(tooltip)
        clearBtn->setToolTip(QCoreApplication::translate("MainWindow", "Clear all files from the list", nullptr));
#endif // QT_CONFIG(tooltip)
        applyAllBtn->setText(QCoreApplication::translate("MainWindow", "Apply All", nullptr));
#if QT_CONFIG(tooltip)
        applyAllBtn->setToolTip(QCoreApplication::translate("MainWindow", "Apply the selected file's settings to all files in the list", nullptr));
#endif // QT_CONFIG(tooltip)
        QTableWidgetItem *___qtablewidgetitem = fileTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "File Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = fileTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Status", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = fileTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Video Codec", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = fileTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Resolution", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = fileTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Frame Rate", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = fileTable->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "Bit Depth", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = fileTable->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "Color Space", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = fileTable->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "Duration", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = fileTable->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "File Size", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = fileTable->horizontalHeaderItem(9);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "Output Name", nullptr));
        settingsGroup->setTitle(QCoreApplication::translate("MainWindow", "Output Settings", nullptr));
        outputFolderLabel->setText(QCoreApplication::translate("MainWindow", "Output Folder:", nullptr));
#if QT_CONFIG(tooltip)
        outputFolderEdit->setToolTip(QCoreApplication::translate("MainWindow", "Directory where processed files will be saved", nullptr));
#endif // QT_CONFIG(tooltip)
        browseBtn->setText(QCoreApplication::translate("MainWindow", "Browse", nullptr));
        formatLabel->setText(QCoreApplication::translate("MainWindow", "Output Format:", nullptr));
        formatCombo->setItemText(0, QCoreApplication::translate("MainWindow", "mp4", nullptr));
        formatCombo->setItemText(1, QCoreApplication::translate("MainWindow", "mkv", nullptr));
        formatCombo->setItemText(2, QCoreApplication::translate("MainWindow", "mov", nullptr));
        formatCombo->setItemText(3, QCoreApplication::translate("MainWindow", "webm", nullptr));
        formatCombo->setItemText(4, QCoreApplication::translate("MainWindow", "ts", nullptr));

#if QT_CONFIG(tooltip)
        formatCombo->setToolTip(QCoreApplication::translate("MainWindow", "Target container format", nullptr));
#endif // QT_CONFIG(tooltip)
        namingLabel->setText(QCoreApplication::translate("MainWindow", "Naming Rule:", nullptr));
        prefixEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Prefix", nullptr));
#if QT_CONFIG(tooltip)
        prefixEdit->setToolTip(QCoreApplication::translate("MainWindow", "Text to add before original filename", nullptr));
#endif // QT_CONFIG(tooltip)
        namingMiddleLabel->setText(QCoreApplication::translate("MainWindow", "[filename]", nullptr));
        suffixEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Suffix", nullptr));
#if QT_CONFIG(tooltip)
        suffixEdit->setToolTip(QCoreApplication::translate("MainWindow", "Text to add after original filename", nullptr));
#endif // QT_CONFIG(tooltip)
        suffixEdit->setText(QCoreApplication::translate("MainWindow", "_muxed", nullptr));
        conflictCombo->setItemText(0, QCoreApplication::translate("MainWindow", "Auto Rename", nullptr));
        conflictCombo->setItemText(1, QCoreApplication::translate("MainWindow", "Overwrite", nullptr));
        conflictCombo->setItemText(2, QCoreApplication::translate("MainWindow", "Skip", nullptr));

#if QT_CONFIG(tooltip)
        conflictCombo->setToolTip(QCoreApplication::translate("MainWindow", "How to handle existing output files", nullptr));
#endif // QT_CONFIG(tooltip)
        compatibilityCheck->setText(QCoreApplication::translate("MainWindow", "Smart Compatibility", nullptr));
#if QT_CONFIG(tooltip)
        compatibilityCheck->setToolTip(QCoreApplication::translate("MainWindow", "Automatically choose compatible container formats", nullptr));
#endif // QT_CONFIG(tooltip)
        sequenceLabel->setText(QCoreApplication::translate("MainWindow", "Sequence ID:", nullptr));
        sequenceEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "A01", nullptr));
#if QT_CONFIG(tooltip)
        sequenceEdit->setToolTip(QCoreApplication::translate("MainWindow", "Sequence identifier (e.g. A01, B02)", nullptr));
#endif // QT_CONFIG(tooltip)
        bitDepthYuvLabel->setText(QCoreApplication::translate("MainWindow", "Bit Depth:", nullptr));
        bitDepthYuvCombo->setItemText(0, QCoreApplication::translate("MainWindow", "8bit", nullptr));
        bitDepthYuvCombo->setItemText(1, QCoreApplication::translate("MainWindow", "10bit", nullptr));
        bitDepthYuvCombo->setItemText(2, QCoreApplication::translate("MainWindow", "12bit", nullptr));

        sceneLabel->setText(QCoreApplication::translate("MainWindow", "Scene Name:", nullptr));
        sceneEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "IndoorTexture", nullptr));
#if QT_CONFIG(tooltip)
        sceneEdit->setToolTip(QCoreApplication::translate("MainWindow", "Scene description name", nullptr));
#endif // QT_CONFIG(tooltip)
        resolutionYuvLabel->setText(QCoreApplication::translate("MainWindow", "Resolution:", nullptr));
        resolutionYuvCombo->setItemText(0, QCoreApplication::translate("MainWindow", "1920x1080", nullptr));
        resolutionYuvCombo->setItemText(1, QCoreApplication::translate("MainWindow", "3840x2160", nullptr));
        resolutionYuvCombo->setItemText(2, QCoreApplication::translate("MainWindow", "2560x1440", nullptr));
        resolutionYuvCombo->setItemText(3, QCoreApplication::translate("MainWindow", "1280x720", nullptr));

        frameRateYuvLabel->setText(QCoreApplication::translate("MainWindow", "Frame Rate:", nullptr));
        frameRateYuvCombo->setItemText(0, QCoreApplication::translate("MainWindow", "30fps", nullptr));
        frameRateYuvCombo->setItemText(1, QCoreApplication::translate("MainWindow", "60fps", nullptr));
        frameRateYuvCombo->setItemText(2, QCoreApplication::translate("MainWindow", "24fps", nullptr));
        frameRateYuvCombo->setItemText(3, QCoreApplication::translate("MainWindow", "25fps", nullptr));

        colorFormatLabel->setText(QCoreApplication::translate("MainWindow", "Color Format:", nullptr));
        colorFormatCombo->setItemText(0, QCoreApplication::translate("MainWindow", "420p", nullptr));
        colorFormatCombo->setItemText(1, QCoreApplication::translate("MainWindow", "yuv420p10le", nullptr));

        previewLabel->setText(QCoreApplication::translate("MainWindow", "Preview: A01_8bit_Scene_1920x1080_30fps_420p.yuv", nullptr));
        previewLabel->setStyleSheet(QCoreApplication::translate("MainWindow", "QLabel { color: #10b981; font-family: monospace; }", nullptr));
        processGroup->setTitle(QCoreApplication::translate("MainWindow", "Processing", nullptr));
        modeLabel->setText(QCoreApplication::translate("MainWindow", "Processing Mode:", nullptr));
        muxingModeRadio->setText(QCoreApplication::translate("MainWindow", "Video Muxing", nullptr));
#if QT_CONFIG(tooltip)
        muxingModeRadio->setToolTip(QCoreApplication::translate("MainWindow", "Standard video file muxing and processing", nullptr));
#endif // QT_CONFIG(tooltip)
        binToYuvModeRadio->setText(QCoreApplication::translate("MainWindow", "BIN -> YUV Conversion", nullptr));
#if QT_CONFIG(tooltip)
        binToYuvModeRadio->setToolTip(QCoreApplication::translate("MainWindow", "Convert BIN files to YUV with proper naming", nullptr));
#endif // QT_CONFIG(tooltip)
        startBtn->setText(QCoreApplication::translate("MainWindow", "Start Processing", nullptr));
#if QT_CONFIG(tooltip)
        startBtn->setToolTip(QCoreApplication::translate("MainWindow", "Begin batch muxing process", nullptr));
#endif // QT_CONFIG(tooltip)
        stopBtn->setText(QCoreApplication::translate("MainWindow", "Stop", nullptr));
#if QT_CONFIG(tooltip)
        stopBtn->setToolTip(QCoreApplication::translate("MainWindow", "Stop current processing", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        progressBar->setToolTip(QCoreApplication::translate("MainWindow", "Overall processing progress", nullptr));
#endif // QT_CONFIG(tooltip)
        statusLabel->setText(QCoreApplication::translate("MainWindow", "Ready", nullptr));
        logGroup->setTitle(QCoreApplication::translate("MainWindow", "Log Output", nullptr));
        infoCheck->setText(QCoreApplication::translate("MainWindow", "Info", nullptr));
        warningCheck->setText(QCoreApplication::translate("MainWindow", "Warning", nullptr));
        errorCheck->setText(QCoreApplication::translate("MainWindow", "Error", nullptr));
        clearLogBtn->setText(QCoreApplication::translate("MainWindow", "Clear Log", nullptr));
#if QT_CONFIG(tooltip)
        logEdit->setToolTip(QCoreApplication::translate("MainWindow", "Processing log with different message levels", nullptr));
#endif // QT_CONFIG(tooltip)
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
