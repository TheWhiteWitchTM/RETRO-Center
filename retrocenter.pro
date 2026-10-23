TEMPLATE = app
TARGET = retrocenter
QT += quick quickcontrols2
CONFIG += c++17 warn_on
CONFIG -= app_bundle

SOURCES += src/main.cpp src/library.cpp
HEADERS += src/library.h
RESOURCES += resources.qrc
INCLUDEPATH += src

# $ORIGIN/../lib is dist/lib when the binary lives in dist/bin. The Qt prefix
# is the fallback so ./build/retrocenter still runs when Qt is installed.
unix:!macx {
    QMAKE_LFLAGS += -Wl,--disable-new-dtags
    QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib\''
    QMAKE_RPATHDIR += $$[QT_INSTALL_LIBS]
}

win32:CONFIG += console
macx:CONFIG -= app_bundle
