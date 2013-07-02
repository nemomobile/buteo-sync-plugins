TEMPLATE = lib
TARGET = hcontacts-changenotifier

DEPENDPATH += .

CONFIG += link_pkgconfig plugin link_pkgconfig

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    PKGCONFIG += buteosyncfw
    MOBILITY += contacts
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG += buteosyncfw5 Qt5Contacts
}

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

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

target.path = /usr/lib/buteo-plugins
INSTALLS += target
