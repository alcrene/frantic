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
    integrator.tpp \
    history.tpp \
    histcollection.tpp \
    io.cpp \

HEADERS += \
    integrator.h \
    history.h \
    euler.h \
    euler_sttic.h \
    rkf45_gsl.h \
    histcollection.h \
    stochastic.h \
    io.h

LIBS += -L$$(HOME)/usr/lib   # The .o files in /build need to be able to find libFRANTIC

# O2scl library

LIBS += -lo2scl
