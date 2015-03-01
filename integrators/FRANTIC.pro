#-------------------------------------------------
#
# FRANTIC-core project file created 2014-03-25
#
#-------------------------------------------------

TARGET = FRANTIC
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++11

DEFINES += O2SCL_CPP11

SOURCES += \
    histcollection.tpp \
    io.cpp \
    integrator.tpp \
    history.tpp

HEADERS += \
    euler.h \
    euler_sttic.h \
    rkf45_gsl.h \
    histcollection.h \
    io.h \
    integrator.h \
    stochastic.h \
    history.h

# O2scl library

LIBS += -lo2scl
