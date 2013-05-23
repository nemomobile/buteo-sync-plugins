TEMPLATE = lib
TARGET = hcalendar-storage
DEPENDPATH += . \
              
INCLUDEPATH += . \
    ../../syncmlcommon
		
		
CONFIG += link_pkgconfig plugin mkcal
PKGCONFIG += buteosyncfw libkcalcoren

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

QT -= gui

LIBS += -L../../syncmlcommon -lsyncmlcommon

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

target.path = /usr/lib/buteo-plugins

ctcaps.path =/etc/buteo/xml/
ctcaps.files=xml/CTCaps_calendar_11.xml xml/CTCaps_calendar_12.xml

INSTALLS += target ctcaps
QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno $(OBJECTS_DIR)/*.gcov $(OBJECTS_DIR)/moc_*

