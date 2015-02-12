TEMPLATE = lib
TARGET = hcontacts-storage

DEPENDPATH += .
INCLUDEPATH += .  \
    ../../syncmlcommon

CONFIG += link_pkgconfig plugin

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    PKGCONFIG = buteosyncfw qtcontacts-sqlite-extensions
    LIBS += -lsyncmlcommon
    MOBILITY += contacts versit
    target.path = /usr/lib/buteo-plugins
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG = buteosyncfw5 Qt5Contacts Qt5Versit qtcontacts-sqlite-qt5-extensions
    LIBS += -lsyncmlcommon5
    target.path = /usr/lib/buteo-plugins-qt5
}

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

QT += gui sql

HEADERS += ContactsStorage.h \
           ContactsBackend.h \
           ContactsImport.h \
           ContactPropertyHandler.h

SOURCES += ContactsStorage.cpp \
           ContactsBackend.cpp \
           ContactsImport.cpp \
           ContactPropertyHandler.cpp


QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

LIBS += -L../../syncmlcommon

QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)

ctcaps.path =/etc/buteo/xml/
ctcaps.files=xml/CTCaps_contacts_11.xml xml/CTCaps_contacts_12.xml

INSTALLS += target ctcaps
