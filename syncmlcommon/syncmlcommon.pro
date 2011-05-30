TEMPLATE = lib
TARGET = syncmlcommon
DEPENDPATH += .
INCLUDEPATH += . \
               /usr/include/libsynccommon
               
LIBS += -lsyncpluginmgr -lmeegosyncml -lsyncprofile

CONFIG += create_pc create_prl mobility

MOBILITY += systeminfo
QT += sql xml
QT -= gui

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

# for compiling on meego
linux-g++-maemo {
  message("Compiling with sysinfo support")
  DEFINES += __SYSINFO__
} else {
  message("Compiling without sysinfo")
}
