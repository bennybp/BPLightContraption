#-------------------------------------------------
#
# Project created by QtCreator 2013-05-30T22:15:31
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BPLightContraption
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11


SOURCES += \
    triaclight.cpp \
    powerunit_gui.cpp \
    powerunit.cpp \
    microcontexception.cpp \
    microcont.cpp \
    main.cpp

HEADERS  += \
    triaclight.h \
    powerunit_gui.h \
    powerunit.h \
    microcontexception.h \
    microcont.h \
    commands-text.h \
    commands.h

FORMS    += \
    triaclight.ui
