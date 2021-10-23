QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
include(../../../g/g.pri)
INCLUDEPATH *= ../../src
DESTDIR = ../../bin

SOURCES += \
	../../src/etharppacket.cpp \
	../../src/fullscan.cpp \
	../../src/host.cpp \
	../../src/livehostmgr.cpp \
	../../src/oldhostmgr.cpp \
	livehostmgr-test.cpp

HEADERS += \
	../../src/etharppacket.h \
	../../src/fullscan.h \
	../../src/host.h \
	../../src/livehostmgr.h \
	../../src/oldhostmgr.h \
	livehostmgr-test.h
