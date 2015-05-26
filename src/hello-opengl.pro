QT += core gui opengl

TARGET = hello-opengl

TEMPLATE = app

SOURCES += main.cpp\
           glwidget.cpp

HEADERS += glwidget.h

OTHER_FILES +=\
               vertexShader.vsh\
               toy.fsh

RESOURCES += resources.qrc
