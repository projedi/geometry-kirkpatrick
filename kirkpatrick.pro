TEMPLATE = app
TARGET = kirkpatrick

CONFIG += QtGui
QT += opengl

OBJECTS_DIR = bin

QMAKE_CXXFLAGS = -g -std=c++11 -Wall

macx {
    QMAKE_CXXFLAGS += -stdlib=libc++  
    QMAKE_LFLAGS += -lc++
}

DEPENDPATH += src \
              visualization/headers \
              visualization/headers/common \
              visualization/headers/io \
              visualization/headers/visualization \

INCLUDEPATH += src \
               visualization/headers \

HEADERS += src/graph.h \
           src/kirkpatrick.h \
           src/viewer.h

SOURCES += src/graph.cpp \
           src/kirkpatrick.cpp \
           src/main.cpp \
           src/viewer.cpp

LIBS += -Lvisualization -lvisualization
