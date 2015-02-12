TEMPLATE = lib
TARGET = hcontacts-storage

DEPENDPATH += .
INCLUDEPATH += .  \
    ../../syncmlcommon

CONFIG += link_pkgconfig plugin

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    PKGCONFIG = buteosyncfw qtcontacts-sqlite-extensions contactcache
    LIBS += -lsyncmlcommon
    MOBILITY += contacts versit
    target.path = /usr/lib/buteo-plugins
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG = buteosyncfw5 Qt5Contacts Qt5Versit qtcontacts-sqlite-qt5-extensions contactcache-qt5
    LIBS += -lsyncmlcommon5
    target.path = /usr/lib/buteo-plugins-qt5
}

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

QT += sql gui

HEADERS += ContactsStorage.h \
           ContactsBackend.h \
           ContactDetailHandler.h

SOURCES += ContactsStorage.cpp \
           ContactsBackend.cpp \
           ContactDetailHandler.cpp


QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

LIBS += -L../../syncmlcommon

QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)

ctcaps.path =/etc/buteo/xml/
ctcaps.files=xml/CTCaps_contacts_11.xml xml/CTCaps_contacts_12.xml

INSTALLS += target ctcaps
