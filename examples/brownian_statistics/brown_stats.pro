#-------------------------------------------------
#
# Project created by QtCreator 2014-01-03T12:03:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = brownian_statistics
TEMPLATE = app

DEFINES += O2SCL_CPP11

INCLUDEPATH += /usr/include/qwt            # header files
DEPENDPATH += /usr/lib                     # libraries



SOURCES += main.cpp\
    brown_stats.cpp


HEADERS += \
    brown_stats.h

# HDF5 library search path
INCLUDEPATH += /home/alex/local/hdf5/include
LIBS += -L/home/alex/local/hdf5/lib64

#CXXFLAGS += "c++11"
# GCC flag; check for other compilers if required

QMAKE_CXXFLAGS += -std=gnu++0x
# This might be a gcc only flag
#CONFIG(release, debug|release): QMAKE_CXXFLAG += -g -O2

win32:CONFIG(release, debug|release): LIBS += -L/usr/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L/usr/lib/ -lqwtd
else:unix: LIBS += -L/usr/lib/ -lqwt

# CENT library
INCLUDEPATH += /home/alex/code/c++/cent
#DEPENDPATH += /home/alex/local/lib  # Somehow this seems to be ignored
LIBS += -L/home/alex/local/lib


LIBS += -lhdf5

CONFIG(release, debug|release): LIBS += -lCENT -lqtCENT
else:CONFIG(debug, debug|release): LIBS += -lCENT-debug -lqtCENT-debug

# O2scl library

LIBS += -lo2scl -lo2scl_hdf

# CLHEP library components

#INCLUDEPATH += /home/alex/local/include
#DEPENDPATH += /home/alex/local/lib
#DEPENDPATH += /home/alex/local/lib64

#unix:!macx: LIBS += -L/home/alex/local/lib64 -lCLHEP-Random-2.1.4.1
