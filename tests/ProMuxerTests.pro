QT += core widgets testlib

CONFIG += c++17 testcase console
TARGET = ProMuxerTests
TEMPLATE = app

# Source files under test
SOURCES += \
    ../src/core/FileProcessor.cpp \
    ../src/core/MediaAnalyzer.cpp \
    ../src/core/MuxingTask.cpp \
    ../src/ui/MainWindow.cpp \
    ../src/ui/FFmpegSetupDialog.cpp

HEADERS += \
    ../src/core/FileProcessor.h \
    ../src/core/MediaAnalyzer.h \
    ../src/core/MuxingTask.h \
    ../src/ui/MainWindow.h \
    ../src/ui/FFmpegSetupDialog.h

# Test files
SOURCES += \
    TestFFmpegCommandGeneration.cpp \
    TestMediaAnalyzer.cpp \
    TestAutoAnalysis.cpp \
    TestUIWorkflow.cpp \
    TestLogColorAdaptation.cpp \
    TestEndToEndWorkflow.cpp \
    TestRegressionSuite.cpp \
    main.cpp

HEADERS += \
    TestFFmpegCommandGeneration.h \
    TestMediaAnalyzer.h \
    TestAutoAnalysis.h \
    TestUIWorkflow.h \
    TestLogColorAdaptation.h \
    TestEndToEndWorkflow.h \
    TestRegressionSuite.h \
    TestHelper.h

FORMS += \
    ../src/ui/MainWindow.ui

# Include path for source headers
INCLUDEPATH += ../src

# Test data
RESOURCES += testdata.qrc

# Windows specific settings
win32 {
    CONFIG += console
    QMAKE_TARGET_PRODUCT = "ProMuxer Test Suite"
    QMAKE_TARGET_DESCRIPTION = "Comprehensive Test Suite for ProMuxer"
}