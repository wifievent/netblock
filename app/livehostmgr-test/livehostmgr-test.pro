QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
include(../../../g/g.pri)
INCLUDEPATH *= ../netblock
DESTDIR = ../../bin

SOURCES += \
	../netblock/host.cpp \
	../netblock/livehostmgr.cpp \
	livehostmgr-test.cpp

HEADERS += \
	../netblock/host.h \
	../netblock/livehostmgr.h \
	livehostmgr-test.h
