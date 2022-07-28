QT       += core gui
QT       += network
QT       += webenginewidgets
QT       += widgets

TARGET = "tlv"
TEMPLATE = app

FORMS       = \
    filtertab.ui \
    finddlg.ui \
    highlightdlg.ui \
    logtab.ui \
    mainwindow.ui \
    optionsdlg.ui \
    savefilterdialog.ui \
    valuedlg.ui 

HEADERS     = \
    colorlibrary.h \
    column.h \
    filtertab.h \
    finddlg.h \
    highlightdlg.h \
    highlightoptions.h \
    logtab.h \
    mainwindow.h \
    options.h \
    optionsdlg.h \
    pathhelper.h \
    processevent.h \
    savefilterdialog.h \
    searchopt.h \
    statusbar.h \
    tokenizer.h \
    treeitem.h \
    treemodel.h \
    valuedlg.h \
    zoomabletreeview.h \
    themeutils.h \
    theme.h \
    qjsonutils.h

SOURCES     = \
    colorlibrary.cpp \
    filtertab.cpp \
    finddlg.cpp \
    highlightdlg.cpp \
    highlightoptions.cpp \
    logtab.cpp \
    main.cpp \
    mainwindow.cpp \
    options.cpp \
    optionsdlg.cpp \
    pathhelper.cpp \
    processevent.cpp \
    savefilterdialog.cpp \
    searchopt.cpp \
    statusbar.cpp \
    tokenizer.cpp \
    treeitem.cpp \
    treemodel.cpp \
    valuedlg.cpp \
    zoomabletreeview.cpp \
    themeutils.cpp \
    theme.cpp \
    qjsonutils.cpp

RESOURCES   = resources.qrc

win32:RC_ICONS += ../resources/images/tlv.ico

ICON = ../resources/images/tlv.icns

CONFIG += c++17
CONFIG += x86_64 

QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

VERSION = 1.3.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
