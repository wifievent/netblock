QT       += core gui
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

include(../../../g/g.pri)
INCLUDEPATH *= ../../src
DESTDIR = ../../bin

LIBS *= -L$${PWD}/../../../opensocket/lib -lopensocket
INCLUDEPATH *= $${PWD}/../../../opensocket/external
INCLUDEPATH *= $${PWD}/../../../opensocket/src
PRE_TARGETDEPS *= $${PWD}/../../../opensocket/lib/libopensocket.a

LIBS *= -L$${PWD}/../../../opennet/bin -lOpenNet
INCLUDEPATH *= $${PWD}/../../../opennet/src
PRE_TARGETDEPS *= $${PWD}/../../../opennet/bin/libOpenNet.a

RESOURCES += image.qrc

RC_ICONS += logo.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../src/etharppacket.cpp \
    ../../src/fullscan.cpp \
    ../../src/host.cpp \
    ../../src/livehostmgr.cpp \
    ../../src/oldhostmgr.cpp \
    main.cpp \
    mainwindow.cpp \
    dinfo.cpp \
    netblock.cpp \
    policyconfig.cpp \
    policyobj.cpp \
    weudpclient.cpp \
    weudpserver.cpp

HEADERS += \
    ../../src/etharppacket.h \
    ../../src/fullscan.h \
    ../../src/host.h \
    ../../src/livehostmgr.h \
    ../../src/oldhostmgr.h \
    mainwindow.h \
    dinfo.h \
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
