#-------------------------------------------------
#
# Project created by QtCreator 2021-01-30T12:53:03
#
#-------------------------------------------------

QT += core gui widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tetris-deepin
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/widget.cpp \
    src/tetrisgame.cpp \
    src/gameboard.cpp \
    src/nextpiecewidget.cpp \
    src/soundmanager.cpp

HEADERS  += src/mainwindow.h \
    src/widget.h \
    src/tetrisgame.h \
    src/tetriscolors.h \
    src/gameboard.h \
    src/nextpiecewidget.h \
    src/soundmanager.h

CONFIG += c++17

linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += dtk6widget
}

FORMS += \
    src/widget.ui

RESOURCES += \
    src/resources.qrc

TRANSLATIONS = \
    src/translations/tetris_es.ts \
    src/translations/tetris_fr.ts \
    src/translations/tetris_de.ts \
    src/translations/tetris_pt.ts \
    src/translations/tetris_zh.ts \
    src/translations/tetris_ru.ts \
    src/translations/tetris_uk.ts \
    src/translations/tetris_it.ts

linux {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    target.path = $$PREFIX/bin
    INSTALLS += target

    desktopfile.path = $$PREFIX/share/applications
    desktopfile.files = src/tetris-deepin.desktop
    INSTALLS += desktopfile

    icon.path = $$PREFIX/share/icons/hicolor/scalable/apps
    icon.files = src/icons/tetris-deepin.svg
    INSTALLS += icon
}

