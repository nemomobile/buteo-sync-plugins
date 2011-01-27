TEMPLATE = lib
TARGET = hcalendar-storage
DEPENDPATH += . \
              
INCLUDEPATH += . \
    /usr/include/libsyncpluginmgr \
    /usr/include/libsynccommon \
    /usr/include/sync \
    ../../syncmlcommon
		
		
CONFIG += debug plugin kcalcoren mkcal

QT -= gui

LIBS += -L../../syncmlcommon -lsyncmlcommon -lsyncpluginmgr

HEADERS += CalendarStorage.h \
           definitions.h \
	   CalendarBackend.h \ 

SOURCES += CalendarStorage.cpp \
           CalendarBackend.cpp


QMAKE_CXXFLAGS = -Wall \
    -g \
    -Wno-cast-align \
    -O2 -finline-functions

QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)

target.path = /usr/lib/sync/

ctcaps.path =/etc/sync/xml/
ctcaps.files=xml/CTCaps_calendar_11.xml xml/CTCaps_calendar_12.xml

INSTALLS += target ctcaps
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_*

