TARGET = syncml-server
CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0 buteosyncfw5 buteosyncml5 Qt5SystemInfo

INCLUDEPATH += . ../../syncmlcommon
LIBS += -L../../syncmlcommon
LIBS += -lsyncmlcommon5

QT       -= gui

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_*

SOURCES += SyncMLServer.cpp \
    USBConnection.cpp \
    BTConnection.cpp

HEADERS += SyncMLServer.h\
    syncmlserver_global.h \
    USBConnection.h \
    BTConnection.h

OTHER_FILES += xml/*

PLUGIN_DLL {
    message("building syncml-server as in-process plugin")
    TEMPLATE = lib
    CONFIG += plugin
    DEFINES += SYNCMLSERVER_LIBRARY
    target.path = /usr/lib/buteo-plugins-qt5
}

PLUGIN_EXE {
    message("building syncml-server as out-of-process plugin")
    TEMPLATE = app
    target.path = /usr/lib/buteo-plugins-qt5/oopp/
    DEFINES += "CLASSNAME=SyncMLServer"
    DEFINES += CLASSNAME_H=\\\"SyncMLServer.h\\\"
    DEFINES += GLIB_FD_WATCH
    DEFINES += SYNCMLSERVER_LIBRARY
    INCLUDE_DIR = $$system(pkg-config --cflags buteosyncfw5|cut -f2 -d'I')
    SOURCES += $$INCLUDE_DIR/ButeoPluginIfaceAdaptor.cpp \
        $$INCLUDE_DIR/PluginCbImpl.cpp \
        $$INCLUDE_DIR/PluginServiceObj.cpp \
        $$INCLUDE_DIR/plugin_main.cpp
    HEADERS += $$INCLUDE_DIR/ButeoPluginIfaceAdaptor.h \
        $$INCLUDE_DIR/PluginCbImpl.h \
        $$INCLUDE_DIR/PluginServiceObj.h
}

sync.path = /etc/buteo/profiles/server
sync.files = xml/syncml.xml

template.path = /etc/buteo/profiles/sync
template.files = xml/bt_template.xml

btsrs.path = /etc/buteo/plugins/syncmlserver
btsrs.files = xml/syncml_server_sdp_record.xml xml/syncml_client_sdp_record.xml

INSTALLS += target sync btsrs template
