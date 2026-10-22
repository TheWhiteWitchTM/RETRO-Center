TEMPLATE = app
TARGET = retrocenter
QT += quick quickcontrols2
CONFIG += c++17 warn_on
CONFIG -= app_bundle

SOURCES += src/main.cpp src/library.cpp
HEADERS += src/library.h
RESOURCES += resources.qrc
INCLUDEPATH += src

# The Qt prefix is baked into the libs. RPATH lets a double-clicked binary
# find that same Qt without a wrapper script.
unix:!macx {
    QMAKE_RPATHDIR += $$[QT_INSTALL_LIBS]
    QMAKE_LFLAGS += -Wl,--disable-new-dtags
}

win32:CONFIG += console
macx:CONFIG -= app_bundle
