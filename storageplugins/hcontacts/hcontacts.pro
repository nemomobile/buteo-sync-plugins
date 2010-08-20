TEMPLATE = lib
TARGET = hcontacts-storage

DEPENDPATH += .
INCLUDEPATH += .  \
    /usr/include/libsynccommon \
	/usr/include/libsyncpluginmgr \
	/usr/include/sync/ \
    /usr/include/qt4/QtMobility/ \
    /usr/include/qt4/QtMobility/QtContacts \
    /usr/include/qt4/QtMobility/QtVersit \
	../../syncmlcommon
	
CONFIG += debug plugin silent

#the contacts library is using QPixmap, so has to comment below line
#QT -= gui 
QT += sql


HEADERS += ContactsStorage.h \
           ContactsBackend.h

SOURCES += ContactsStorage.cpp \
           ContactsBackend.cpp

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

LIBS += -L../../syncmlcommon -lsyncmlcommon -lsyncpluginmgr -lQtContacts -lQtVersit

QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
target.path = /usr/lib/sync/

ctcaps.path =/etc/sync/xml/
ctcaps.files=xml/CTCaps_contacts_11.xml xml/CTCaps_contacts_12.xml

INSTALLS += target ctcaps


