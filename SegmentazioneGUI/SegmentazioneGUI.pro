#-------------------------------------------------
#
# Project created by QtCreator 2016-08-01T10:01:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SegmentazioneGUI
TEMPLATE = app

win32
{
    INCLUDEPATH += C:\Opencv\opencv\release2\install\include # giusta
    LIBS += C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_core310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_highgui310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_imgproc310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_objdetect310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_imgcodecs310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_videoio310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_face310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_calib3d310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_features2d310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_xfeatures2d310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_flann310.dll.a \
    C:\Opencv\opencv\release2\install\x86\mingw\lib\libopencv_ml310.dll.a
}

SOURCES += main.cpp\
        mainwindow.cpp \
    mymouseqlabel.cpp \
    globals.cpp \
    cvmatqimage.cpp

HEADERS  += mainwindow.h \
    mymouseqlabel.h \
    globals.h \
    utilita.h \
    includes.h \
    cvmatqimage.h \
    estensioni.h

FORMS    += mainwindow.ui

RESOURCES += \
    icons.qrc
