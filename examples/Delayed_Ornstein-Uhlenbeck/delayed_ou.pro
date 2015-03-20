#-------------------------------------------------
#
# Project created by QtCreator 2014-01-03T12:03:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = delayed_ou
TEMPLATE = app

DEFINES += O2SCL_CPP11

INCLUDEPATH += /usr/include/qwt            # header files


SOURCES +=\
    delayed_ou.cpp


HEADERS += \
    delayed_ou.h \
    ou_process.h \
    standardwindow.h

QMAKE_CXXFLAGS += -std=gnu++0x
# This might be a gcc only flag
#CONFIG(release, debug|release): QMAKE_CXXFLAG += -g -O2

win32:CONFIG(release, debug|release): LIBS += -L/usr/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L/usr/lib/ -lqwtd
else:unix: LIBS += -L/usr/lib/ -lqwt

LIBS += -L/home/alex/usr/local/lib
LIBS += -L/home/alex/usr/local/lib64

# FRANTIC library
INCLUDEPATH += /home/alex/code/c++/frantic    # header files
DEPENDPATH += /home/alex/code/c++/frantic     # Recompile when headers here change
#LIBS += -L/home/alex/code/c++/frantic           # Add to linker path

CONFIG(release, debug|release): LIBS += -lFRANTIC -lqtFRANTIC
else:CONFIG(debug, debug|release): LIBS += -lFRANTIC-debug -lqtFRANTIC-debug

# O2scl library

LIBS += -lo2scl
