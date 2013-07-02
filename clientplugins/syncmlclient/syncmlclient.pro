TEMPLATE = lib
TARGET = syncml-client
DEPENDPATH += .
INCLUDEPATH += . ../../syncmlcommon

CONFIG += link_pkgconfig plugin 

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    MOBILITY += systeminfo
    PKGCONFIG = buteosyncfw buteosyncml
    LIBS += -lsyncmlcommon
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG = buteosyncfw5 buteosyncml5 Qt0SystemInfo
    LIBS += -lsyncmlcommon5
}

LIBS += -L../../syncmlcommon

QT += dbus sql network
QT -= gui

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

#input
HEADERS += SyncMLClient.h BTConnection.h

SOURCES += SyncMLClient.cpp BTConnection.cpp

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
target.path = /usr/lib/buteo-plugins

client.path = /etc/buteo/profiles/client
client.files = xml/syncml.xml

####To Remove Later After Accounts Integration

sync.path = /etc/buteo/profiles/sync
sync.files = xml/sync/*

#service.path = /etc/buteo/profiles/service
#service.files = xml/service/*

storage.path = /etc/buteo/profiles/storage
storage.files = xml/storage/*

INSTALLS += target client sync service storage
