#-------------------------------------------------
#
# Project created by QtCreator 2018-05-12T11:34:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SN8F2288
TEMPLATE = app

macx {
    ICON = images/cpu.icns
    QMAKE_INFO_PLIST = Info.plist
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    MainWindow.cpp \
    Database.cpp \
    DisassemblerTextEdit.cpp \
    DisassemblerHighlighter.cpp \
    DisassemblerBackend.cpp \
    RangesDialog.cpp \
    XrefsDialog.cpp \
    Core.cpp \
    ChipCpu.cpp \
    Config.cpp \
    RegisterTable.cpp \
    sn8f2288_chip.cpp

HEADERS += \
    MainWindow.h \
    sn8f2288_chip.h \
    Database.h \
    Token.h \
    DisassemblerTextEdit.h \
    DisassemblerHighlighter.h \
    DisassemblerBackend.h \
    Utf8Ini/Utf8Ini.h \
    RangesDialog.h \
    XrefsDialog.h \
    Core.h \
    ChipCpu.h \
    Config.h \
    RegisterTable.h

FORMS += \
    MainWindow.ui \
    RangesDialog.ui \
    XrefsDialog.ui
