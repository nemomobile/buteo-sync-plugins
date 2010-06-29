TEMPLATE = lib
TARGET = syncml-client
DEPENDPATH += . 
INCLUDEPATH += . ../../syncmlcommon \
    /usr/include/libsynccommon \
    /usr/include/libsyncprofile \
    /usr/include/sync \

LIBS += -lsyncpluginmgr -lsyncprofile -lmeegosyncml #-lsysinfo

CONFIG += debug plugin silent

QT += dbus sql network
QT -= gui

#input
HEADERS += SyncMLClient.h \
           ../../syncmlcommon/StorageAdapter.h \
           ../../syncmlcommon/ItemAdapter.h \
           ../../syncmlcommon/ItemIdMapper.h \
           ../../syncmlcommon/SyncMLCommon.h \
           ../../syncmlcommon/SyncMLConfig.h \
           ../../syncmlcommon/SyncMLStorageProvider.h \
           ../../syncmlcommon/DeviceInfo.h 


SOURCES += SyncMLClient.cpp \
           ../../syncmlcommon/StorageAdapter.cpp \
           ../../syncmlcommon/ItemAdapter.cpp \
           ../../syncmlcommon/ItemIdMapper.cpp \
	   ../../syncmlcommon/SyncMLConfig.cpp \
	   ../../syncmlcommon/SyncMLStorageProvider.cpp \
           ../../syncmlcommon/DeviceInfo.cpp 

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

#clean
QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_*
#QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
#QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage



#install
target.path = /usr/lib/sync/

client.path = /etc/sync/profiles/client 
client.files = xml/syncml.xml

####To Remove Later After Accounts Integration

sync.path = /etc/sync/profiles/sync
sync.files = xml/sync/*

service.path = /etc/sync/profiles/service
service.files = xml/service/*

storage.path = /etc/sync/profiles/storage
storage.files = xml/storage/*

INSTALLS += target client sync service storage 


