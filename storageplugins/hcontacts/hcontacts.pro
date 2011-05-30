TEMPLATE = lib
TARGET = hcontacts-storage

linux-g++-maemo {
  message("Compiling for maemo")
} else {
  message("Compiling for non-maemo")
  INCLUDEPATH += \
    /usr/include/QtMobility/ \
    /usr/include/QtContacts \
    /usr/include/QtVersit \
}

DEPENDPATH += .
INCLUDEPATH += .  \
    /usr/include/libsynccommon \
    /usr/include/libsyncpluginmgr \
    /usr/include/sync/ \
    ../../syncmlcommon

CONFIG += plugin mobility
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

LIBS += -L../../syncmlcommon -lsyncmlcommon -lsyncpluginmgr

QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
target.path = /usr/lib/sync/

ctcaps.path =/etc/sync/xml/
ctcaps.files=xml/CTCaps_contacts_11.xml xml/CTCaps_contacts_12.xml

INSTALLS += target ctcaps
