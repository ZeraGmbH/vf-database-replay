TEMPLATE = app
TARGET = vf-database-replay

#dependencies
VEIN_DEP_EVENT = 1
VEIN_DEP_COMP = 1
VEIN_DEP_HASH = 1
VEIN_DEP_TCP2 = 1
VEIN_DEP_NET2 = 1
VEIN_DEP_HELPER = 1

exists( ../../vein-framework.pri ) {
  include(../../vein-framework.pri)
}

QT += core sql network
QT -= gui

CONFIG += c++11

CONFIG += console
CONFIG -= app_bundle



SOURCES += main.cpp \
    ecsdataset.cpp \
    databasereplaysystem.cpp

HEADERS += \
    ecsdataset.h \
    databasereplaysystem.h
