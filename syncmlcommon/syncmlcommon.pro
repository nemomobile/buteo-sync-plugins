TEMPLATE = lib
TARGET = syncmlcommon
DEPENDPATH += .

CONFIG += link_pkgconfig create_pc create_prl mobility
PKGCONFIG = buteosyncfw buteosyncml

MOBILITY += systeminfo
QT += sql xml
QT -= gui

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

#input
HEADERS += ItemAdapter.h \
           ItemIdMapper.h \
           SimpleItem.h \
           StorageAdapter.h \
           SyncMLCommon.h \
           SyncMLConfig.h \
           SyncMLStorageProvider.h \
           FolderItemParser.h \
           DeviceInfo.h

SOURCES += ItemAdapter.cpp \
           ItemIdMapper.cpp \
           SimpleItem.cpp \
           StorageAdapter.cpp \
           SyncMLConfig.cpp \
           SyncMLStorageProvider.cpp \
           FolderItemParser.cpp  \
           DeviceInfo.cpp

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

#clean
QMAKE_CLEAN += $(TARGET)
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_* lib$${TARGET}.prl pkgconfig/*

#install
target.path = /usr/lib/
headers.path = /usr/include/syncmlcommon/
headers.files = ItemAdapter.h \
           ItemIdMapper.h \
           SimpleItem.h \
           StorageAdapter.h \
           SyncMLCommon.h \
           SyncMLConfig.h \
           SyncMLStorageProvider.h \
           FolderItemParser.h \
           DeviceInfo.h

INSTALLS += target headers

QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR  = $$target.path
QMAKE_PKGCONFIG_INCDIR  = $$headers.path
pkgconfig.files = $${TARGET}.pc
