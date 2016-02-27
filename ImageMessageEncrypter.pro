#-------------------------------------------------
#
# Project created by QtCreator 2016-01-13T18:44:30
#
#-------------------------------------------------

QT += core gui widgets
qtHaveModule(printsupport): QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageMessageEncrypter
TEMPLATE = app


SOURCES += main.cpp \
    imageviewer.cpp \
    encodingtoolbox.cpp \
    qlabelclickable.cpp \
    stenographyutils.cpp

HEADERS  += \
    imageviewer.h \
    encodingtoolbox.h \
    qlabelclickable.h \
    stenographyutils.h

wince*: {
   DEPLOYMENT_PLUGIN += qjpeg qgif
}

FORMS += \
    encodingtoolbox.ui

DISTFILES += \
    LICENSE
