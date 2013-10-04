#-------------------------------------------------
#
# Project created by QtCreator 2013-07-08T08:45:57
#
#-------------------------------------------------

QT       -= gui

TARGET = syncml-server
TEMPLATE = lib

CONFIG += link_pkgconfig plugin

DEFINES += GLIB_FD_WATCH

INCLUDEPATH += . ../../syncmlcommon
LIBS += -L../../syncmlcommon
PKGCONFIG += glib-2.0

equals(QT_MAJOR_VERSION, 4): {
    PKGCONFIG += buteosyncfw buteosyncml QtSystemInfo
    LIBS += -lsyncmlcommon
    target.path = /usr/lib/buteo-plugins
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG += buteosyncfw5 buteosyncml5 Qt5SystemInfo
    LIBS += -lsyncmlcommon5
    target.path = /usr/lib/buteo-plugins-qt5
}

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

DEFINES += SYNCMLSERVER_LIBRARY

SOURCES += SyncMLServer.cpp \
    USBConnection.cpp \
    BTConnection.cpp

HEADERS += SyncMLServer.h\
        syncmlserver_global.h \
    USBConnection.h \
    BTConnection.h


#cleanup
QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_*

sync.path = /etc/buteo/profiles/server
sync.files = xml/syncml.xml

template.path = /etc/buteo/profiles/sync
template.files = xml/bt_template.xml

btsrs.path = /etc/buteo/plugins/syncmlserver
btsrs.files = xml/syncml_server_sdp_record.xml xml/syncml_client_sdp_record.xml

#installs
INSTALLS += target sync btsrs template

OTHER_FILES += xml/syncml_server_sdp_record.xml \
                xml/syncml_client_sdp_record.xml
