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



HEADERS += tab.h \
    curve.h \
    plot.h \
    histogram.h \
    qtfrantic.h


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

#INCLUDEPATH += /home/alex/code/c++/frantic
#DEPENDPATH += /home/alex/code/c++/frantic/builds/debug

#unix:!macx: LIBS += -L/home/alex/local/lib64

# O2scl library

LIBS += -lo2scl

