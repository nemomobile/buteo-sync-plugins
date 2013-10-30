# Configuration for generation a DLL plugin
PLUGIN_LIBRARY {

TEMPLATE = lib
TARGET = syncml-client
DEPENDPATH += .
INCLUDEPATH += . ../../syncmlcommon

CONFIG += link_pkgconfig plugin

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    MOBILITY += systeminfo
    PKGCONFIG = buteosyncfw buteosyncml accounts-qt libsignon-qt
    LIBS += -lsyncmlcommon
    target.path = /usr/lib/buteo-plugins
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG = buteosyncfw5 buteosyncml5 Qt5SystemInfo accounts-qt5 libsignon-qt5
    LIBS += -lsyncmlcommon5
    target.path = /usr/lib/buteo-plugins-qt5
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

}

# Configuration for generation an out-of-process plugin
PLUGIN_EXE {

TEMPLATE = app
TARGET = syncml-client
DEPENDPATH += .
INCLUDEPATH += . ../../syncmlcommon

CONFIG += link_pkgconfig plugin

PKGCONFIG = buteosyncfw5 buteosyncml5 Qt5SystemInfo accounts-qt5 libsignon-qt5
LIBS += -lsyncmlcommon5
target.path = /usr/lib/buteo-plugins-qt5/oopp

DEFINES += "CLASSNAME=SyncMLClient"
DEFINES += CLASSNAME_H=\\\"SyncMLClient.h\\\"

LIBS += -L../../syncmlcommon

QT += dbus sql network
QT -= gui

INCLUDE_DIR = $$system(pkg-config --cflags buteosyncfw5|cut -f2 -d'I')

#input
HEADERS += SyncMLClient.h \
           BTConnection.h \
           $$INCLUDE_DIR/ButeoPluginIfAdaptor.h \
           $$INCLUDE_DIR/PluginServiceObj.h

SOURCES += SyncMLClient.cpp \
           BTConnection.cpp \
           $$INCLUDE_DIR/ButeoPluginIfAdaptor.cpp \
           $$INCLUDE_DIR/PluginServiceObj.cpp \
           $$INCLUDE_DIR/plugin_main.cpp

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

client.path = /etc/buteo/profiles/client
client.files = xml/syncml.xml

sync.path = /etc/buteo/profiles/sync
sync.files = xml/sync/*

storage.path = /etc/buteo/profiles/storage
storage.files = xml/storage/*

INSTALLS += target client sync storage

}
