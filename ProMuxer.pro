QT += core widgets

CONFIG += c++17

TARGET = ProMuxer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/ui/MainWindow.cpp \
    src/core/FileProcessor.cpp \
    src/core/MuxingTask.cpp \
    src/core/MediaAnalyzer.cpp

HEADERS += \
    src/ui/MainWindow.h \
    src/core/FileProcessor.h \
    src/core/MuxingTask.h \
    src/core/MediaAnalyzer.h

FORMS += \
    src/ui/MainWindow.ui

# Windows specific settings
win32 {
    RC_FILE = resources/app.rc
    CONFIG += windows
    VERSION = 1.0.0
    QMAKE_TARGET_PRODUCT = "Pro Muxer"
    QMAKE_TARGET_DESCRIPTION = "Professional Video Stream Muxing Tool"
    QMAKE_TARGET_COPYRIGHT = "Pro Muxer Project"
}

# macOS specific settings
macx {
    ICON = resources/app.icns
    QMAKE_INFO_PLIST = resources/Info.plist
}

# Linux specific settings
unix:!macx {
    TARGET = promuxer
    target.path = /usr/local/bin
    INSTALLS += target
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target