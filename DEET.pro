#-------------------------------------------------
#
# Project created by QtCreator 2013-09-23T12:39:47
#
#-------------------------------------------------

TARGET = DEET
TEMPLATE = lib
CONFIG += staticlib

#DEFINES += DEET_LIBRARY

SOURCES += deet.cpp \
    solvers/dummy.tpp \
    solvers/euler.tpp \
    solvers/RK4.tpp \
    solvers/solver.tpp \
    ui/dtbcurve.cpp \
    ui/dtbplot.cpp \
    ui/tab.cpp

HEADERS += deet.h\
    solvers/dummy.h \
    solvers/euler.h \
    solvers/RK4.h \
    solvers/solver.h \
    ui/dtbcurve.h \
    ui/dtbplot.h \
    ui/tab.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
