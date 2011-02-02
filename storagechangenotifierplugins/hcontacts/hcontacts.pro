TEMPLATE = lib
TARGET = hcontacts-changenotifier

DEPENDPATH += .
INCLUDEPATH += . \
               /usr/include/libsyncpluginmgr \
               /usr/include/libsynccommon

CONFIG += debug plugin mobility 
MOBILITY += contacts
 
QT -= GUI

HEADERS += ContactsChangeNotifierPlugin.h \
           ContactsChangeNotifier.h

SOURCES += ContactsChangeNotifierPlugin.cpp \
           ContactsChangeNotifier.cpp

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

QMAKE_CLEAN += $(TARGET)

target.path = /usr/lib/sync/
INSTALLS += target
