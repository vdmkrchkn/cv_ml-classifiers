#-------------------------------------------------
#
# Project created by QtCreator 2015-05-07T16:54:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cvMLClassifier
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    train-dialog.cpp

HEADERS  += mainwindow.h \
    train-dialog.h

FORMS    += mainwindow.ui \
    train-dialog.ui

INCLUDEPATH += c:\\Programs\\Qt\\Qt5.4.0\\Tools\\opencv-2.4.10\\install\\include
INCLUDEPATH += c:\\Programs\\Qt\\Qt5.4.0\\Tools\\opencv-2.4.10\\install\\include\\opencv

LIBS += -LC:\\Programs\\Qt\\Qt5.4.0\\Tools\\opencv-2.4.10\\install\\x64\\mingw\\lib \
	-lopencv_core2410.dll \
	-lopencv_highgui2410.dll \
	-lopencv_imgproc2410.dll \
	-lopencv_features2d2410.dll \
	-lopencv_calib3d2410.dll \
	-lopencv_ml2410.dll
