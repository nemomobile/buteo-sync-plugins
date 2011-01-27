# -------------------------------------------------
# Project created by QtCreator 2009-07-07T08:59:55
# -------------------------------------------------
QT +=  network \
    script \
    xml \
        testlib
QT -= gui
TARGET = syncmlclientplugtest
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
	TestSyncmlClient.cpp
HEADERS += TestSyncmlClient.h
INCLUDEPATH += ../ \
    /usr/include/sync/ \ 

QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
	
#install
target.path = /usr/share/sync-app-tests/
INSTALLS += target 

    

