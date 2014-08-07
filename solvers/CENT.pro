#-------------------------------------------------
#
# CENT-core project file created 2014-03-25
#
#-------------------------------------------------

TARGET = CENT
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++11

DEFINES += O2SCL_CPP11

SOURCES += solver.tpp \
    series.tpp \
    histcollection.tpp \
    io.cpp

HEADERS += solver.h \
    series.h \
    euler.h \
    euler_sttic.h \
    rkf45_gsl.h \
    histcollection.h \
    io.h

#unix:!symbian {
#    maemo5 {
#        target.path = /opt/usr/lib
#    } else {
#        target.path = /usr/lib
#    }
#    INSTALLS += target
#}

# CLHEP library components

#INCLUDEPATH += /home/alex/local/include
#DEPENDPATH += /home/alex/local/include

#unix:!macx: LIBS += -L/home/alex/local/lib64

# O2scl library

LIBS += -lo2scl
