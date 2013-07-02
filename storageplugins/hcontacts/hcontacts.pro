TEMPLATE = lib
TARGET = hcontacts-storage

DEPENDPATH += .
INCLUDEPATH += .  \
    ../../syncmlcommon

CONFIG += link_pkgconfig plugin 

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    PKGCONFIG = buteosyncfw
    LIBS += -lsyncmlcommon
    MOBILITY += contacts versit
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG = buteosyncfw5 Qt5Contacts Qt5Versit
    LIBS += -lsyncmlcommon5
}

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

#the contacts library is using QPixmap, so has to comment below line
#QT -= gui
QT += sql

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
target.path = /usr/lib/buteo-plugins

ctcaps.path =/etc/buteo/xml/
ctcaps.files=xml/CTCaps_contacts_11.xml xml/CTCaps_contacts_12.xml

INSTALLS += target ctcaps
