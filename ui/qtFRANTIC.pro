#-------------------------------------------------
#
# qtFRANTIC project file created 2014-03-25
#
#-------------------------------------------------

TARGET = qtFRANTIC
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++11

DEFINES += O2SCL_CPP11

SOURCES += tab.cpp \
    curve.cpp \
    plot.cpp \
    histogram.cpp \
#    uiparameter.cpp



HEADERS += tab.h \
    curve.h \
    plot.h \
    histogram.h \
    qtfrantic.h \
    uiparameter.h

INCLUDEPATH += /usr/include/qt4/QtGui
LIBS += -L/home/rene/usr/lib   # The .o files in /build need to be able to find libqtFRANTIC
#INCLUDEPATH += /usr/include/qt4/

# O2scl library

LIBS += -lo2scl

