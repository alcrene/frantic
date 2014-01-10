#-------------------------------------------------
#
# Project created by QtCreator 2013-09-23T12:39:47
#
#-------------------------------------------------

TARGET = DEET
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++11

#DEFINES += DEET_LIBRARY
DEFINES += O2SCL_CPP11

SOURCES += deet.cpp \
    solvers/solver.tpp \
#    solvers/dummy.tpp \
    solvers/euler.tpp \
#    solvers/RK4.tpp \
#    solvers/euler_sttic.tpp \
    ui/dtbcurve.cpp \
    ui/dtbplot.cpp \
    ui/tab.cpp \
    ui/histogram.cpp \
#    solvers/rkf45_gsl.tpp
#    solvers/stochastic.tpp


HEADERS += deet.h\
    solvers/solver.h \
#    solvers/dummy.h \
    solvers/euler.h \
#    solvers/RK4.h \
#    solvers/euler_sttic.h \
#    solvers/rkf45_gsl.h \
#    solvers/noise.h \
    ui/dtbcurve.h \
    ui/dtbplot.h \
    ui/tab.h \
    ui/histogram.h \
    solvers/rkf45_gsl.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

# CLHEP library components

#INCLUDEPATH += /home/alex/local/include
#DEPENDPATH += /home/alex/local/include

#unix:!macx: LIBS += -L/home/alex/local/lib64 -lCLHEP-Random

# O2scl library

LIBS += -lo2scl

