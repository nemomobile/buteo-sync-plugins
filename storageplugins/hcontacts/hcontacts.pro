TEMPLATE = lib
TARGET = hcontacts-storage

DEPENDPATH += .
INCLUDEPATH += .  \
    ../../syncmlcommon

CONFIG += link_pkgconfig plugin mobility
PKGCONFIG += buteosyncfw

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

MOBILITY += contacts versit   
 
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

LIBS += -L../../syncmlcommon -lsyncmlcommon

QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
target.path = /usr/lib/buteo-plugins

ctcaps.path =/etc/buteo/xml/
ctcaps.files=xml/CTCaps_contacts_11.xml xml/CTCaps_contacts_12.xml

INSTALLS += target ctcaps
