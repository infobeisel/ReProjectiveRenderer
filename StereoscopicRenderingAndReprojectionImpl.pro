QT     += core gui
CONFIG += c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5 or later)


TARGET = StereoscopicRenderingAndReprojectionImpl
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    canonicalglwindowimpl.cpp \
    scenemanager.cpp \
    scene.cpp \
    clienttexturearraymanager.cpp \
    clienttexturearray.cpp \
    clienttexture.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../assimp/lib/release/ -lassimp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../assimp/lib/debug/ -lassimp
else:unix: LIBS += -L$$PWD/../../assimp/lib/ -lassimp

INCLUDEPATH += $$PWD/../../assimp/include
DEPENDPATH += $$PWD/../../assimp/include

RESOURCES += \
    resources.qrc

HEADERS += \
    canonicalglwindowimpl.h \
    scenemanager.h \
    scene.h \
    clienttexturearraymanager.h \
    clienttexturearray.h \
    clienttexture.h
