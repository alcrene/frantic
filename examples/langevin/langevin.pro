#-------------------------------------------------
#
# Project created by QtCreator 2014-01-03T12:03:00
# 
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = langevin
TEMPLATE = app

DEFINES += O2SCL_CPP11

INCLUDEPATH += /usr/include/qwt            # header files
DEPENDPATH += /usr/lib                     # libraries



SOURCES += main.cpp\
    langevin.cpp \


HEADERS += \
    langevin.h \


#CXXFLAGS += "c++11"
# GCC flag; check for other compilers if required

QMAKE_CXXFLAGS += -std=gnu++0x
# This might be a gcc only flag

win32:CONFIG(release, debug|release): LIBS += -L/usr/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L/usr/lib/ -lqwtd
else:unix: LIBS += -L/usr/lib/ -lqwt

# DEET library
INCLUDEPATH += /home/alex/code/c++/cent/src
DEPENDPATH += /home/alex/code/c++/cent/builds/debug


unix:!macx: LIBS += -L/home/alex/code/c++/cent/builds/debug/ -lCENT
#unix:!macx: LIBS += -lCENT

# O2scl library

LIBS += -lo2scl

# CLHEP library components

#INCLUDEPATH += /home/alex/local/include
#DEPENDPATH += /home/alex/local/lib
#DEPENDPATH += /home/alex/local/lib64

#unix:!macx: LIBS += -L/home/alex/local/lib64 -lCLHEP-Random-2.1.4.1
