QT       += core gui
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

include(../../../opennet/opennet.pri)
INCLUDEPATH *= ../../src
DESTDIR = ../../bin

LIBS *= -lglog

LIBS *= -ldl


LIBS *= -L$${PWD}/../../../opensocket/lib -lopensocket
INCLUDEPATH *= $${PWD}/../../../opensocket/external
INCLUDEPATH *= $${PWD}/../../../opensocket/src
PRE_TARGETDEPS *= $${PWD}/../../../opensocket/lib/libopensocket.a

RESOURCES += image.qrc

RC_ICONS += logo.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../src/dbconnect.cpp \
    ../../src/sqlite/sqlite3.c \
    ../../src/fullscan.cpp \
    ../../src/host.cpp \
    ../../src/livehostmgr.cpp \
    ../../src/oldhostmgr.cpp \
    main.cpp \
    dinfo.cpp \
    mainwindow.cpp \
    netblock.cpp \
    policyconfig.cpp \
    policyobj.cpp \
    weudpclient.cpp \
    weudpserver.cpp

HEADERS += \
    ../../src/dbconnect.h \
    ../../src/sqlite/sqlite3.h \
    ../../src/fullscan.h \
    ../../src/host.h \
    ../../src/livehostmgr.h \
    ../../src/oldhostmgr.h \
    dinfo.h \
    mainwindow.h \
    netblock.h \
    policyconfig.h \
    policyobj.h \
    weudpclient.h \
    weudpserver.h

FORMS += \
    mainwindow.ui \
    policyconfig.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
