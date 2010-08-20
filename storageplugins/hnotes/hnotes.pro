TEMPLATE = lib
TARGET = hnotes-storage
DEPENDPATH += .
INCLUDEPATH += . ../../syncmlcommon \
    /usr/include/libsynccommon \
    /usr/include/sync \
    /usr/include/kcal

LIBS += -L../../syncmlcommon -lsyncmlcommon -lsyncpluginmgr -lextendedkcal -lQtTest

CONFIG += debug plugin silent

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
target.path = /usr/lib/sync/

ctcaps.path =/etc/sync/xml/
ctcaps.files=xml/CTCaps_notes_11.xml xml/CTCaps_notes_12.xml

INSTALLS += target ctcaps
