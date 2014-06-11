TARGET = syncml-client
DEPENDPATH += .
INCLUDEPATH += . ../../syncmlcommon

CONFIG += link_pkgconfig
PKGCONFIG = buteosyncfw5 buteosyncml5 Qt5SystemInfo accounts-qt5 libsignon-qt5
LIBS += -lsyncmlcommon5
LIBS += -L../../syncmlcommon

QT += dbus sql network
QT -= gui

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

#DEFINES += BUTEO_ENABLE_DEBUG
HEADERS += SyncMLClient.h BTConnection.h
SOURCES += SyncMLClient.cpp BTConnection.cpp

PLUGIN_DLL {
    TEMPLATE = lib
    CONFIG += plugin
    target.path = /usr/lib/buteo-plugins-qt5
}

PLUGIN_EXE {
    TEMPLATE = app
    target.path = /usr/lib/buteo-plugins-qt5/oopp

    DEFINES += "CLASSNAME=SyncMLClient"
    DEFINES += CLASSNAME_H=\\\"SyncMLClient.h\\\"
    DEFINES += CLIENT_PLUGIN

    INCLUDE_DIR = $$system(pkg-config --cflags buteosyncfw5|cut -f2 -d'I')
    HEADERS += $$INCLUDE_DIR/ButeoPluginIfaceAdaptor.h \
               $$INCLUDE_DIR/PluginCbImpl.h \
               $$INCLUDE_DIR/PluginServiceObj.h
    SOURCES += $$INCLUDE_DIR/ButeoPluginIfaceAdaptor.cpp \
               $$INCLUDE_DIR/PluginCbImpl.cpp \
               $$INCLUDE_DIR/PluginServiceObj.cpp \
               $$INCLUDE_DIR/plugin_main.cpp
}

OTHER_FILES += xml/* \
               xml/sync/* \
               xml/service/* \
               xml/storage/*

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_*

client.path = /etc/buteo/profiles/client
client.files = xml/syncml.xml

sync.path = /etc/buteo/profiles/sync
sync.files = xml/sync/*

service.path = /etc/buteo/profiles/service
service.files = xml/service/*

storage.path = /etc/buteo/profiles/storage
storage.files = xml/storage/*

INSTALLS += target client sync service storage
