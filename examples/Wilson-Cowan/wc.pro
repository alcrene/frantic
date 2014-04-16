#-------------------------------------------------
#
# Project created by QtCreator 2014-01-03T12:03:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wilson_cowan
TEMPLATE = app

DEFINES += O2SCL_CPP11

INCLUDEPATH += /usr/include/qwt            # header files
INCLUDEPATH += /home/alex/local/includeB
LIBS += -L/home/alex/local/lib -L/home/alex/local/lib64  # personal library directories


SOURCES +=\
    wc.cpp


HEADERS += \
    wc.h \
    differential.h


QMAKE_CXXFLAGS += -std=gnu++0x
# This might be a gcc only flag
#CONFIG(release, debug|release): QMAKE_CXXFLAG += -g -O2   # Should help profiling, but didn't seem to do much

win32:CONFIG(release, debug|release): LIBS += -L/usr/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L/usr/lib/ -lqwtd
else:unix: LIBS += -L/usr/lib/ -lqwt

# CENT library
INCLUDEPATH += /home/alex/code/c++/cent


CONFIG(release, debug|release): LIBS += -lCENT -lqtCENT
else:CONFIG(debug, debug|release): LIBS += -lCENT-debug -lqtCENT-debug

# O2scl library
LIBS += -lo2scl
