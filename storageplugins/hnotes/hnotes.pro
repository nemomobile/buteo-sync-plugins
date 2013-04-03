TEMPLATE = lib
TARGET = hnotes-storage
DEPENDPATH += .
INCLUDEPATH += . ../../syncmlcommon

LIBS += -L../../syncmlcommon -lsyncmlcommon

CONFIG += link_pkgconfig plugin kcalcoren mkcal
PKGCONFIG += buteosyncfw

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

QT -= gui

#input
HEADERS += NotesStorage.h \
           NotesBackend.h \

SOURCES += NotesStorage.cpp \
           NotesBackend.cpp \

QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions


#clean
QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_*

#install
target.path = /usr/lib/buteo/

ctcaps.path =/etc/buteo/xml/
ctcaps.files=xml/CTCaps_notes_11.xml xml/CTCaps_notes_12.xml

INSTALLS += target ctcaps
