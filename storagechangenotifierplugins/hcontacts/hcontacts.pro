TEMPLATE = lib
TARGET = hcontacts-changenotifier

DEPENDPATH += .
INCLUDEPATH += . \
               /usr/include/buteosyncfw

CONFIG += plugin mobility link_pkgconfig
MOBILITY += contacts
PKGCONFIG = buteosyncfw
 
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

target.path = /usr/lib/
INSTALLS += target
