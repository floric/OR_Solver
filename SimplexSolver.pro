#-------------------------------------------------
#
# Project created by QtCreator 2016-01-15T20:14:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SimplexSolver
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    solver.cpp

HEADERS  += mainwindow.h \
    solver.h

FORMS    += mainwindow.ui

INCLUDEPATH += libs\armadillo-6.400.3\include
LIBS += -Llibs\armadillo-6.400.3\examples\lib_win64
