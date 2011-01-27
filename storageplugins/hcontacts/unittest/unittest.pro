TEMPLATE = app
TARGET = hcontacts-tests 
 
QT += core testlib sql
CONFIG += qtestlib mobility

DEPENDPATH += . \
              ../ \
              ../../../syncmlcommon

linux-g++-maemo {
  message("Compiling for maemo")
} else {
  message("Compiling for non-maemo")
  INCLUDEPATH += \
    /usr/include/QtMobility/ \
    /usr/include/QtContacts \
    /usr/include/QtVersit \
}
              
INCLUDEPATH += . \
    ../ \
    /usr/include/libsynccommon \
    /usr/include/libsyncpluginmgr \
    /usr/include/sync \
    ../../../syncmlcommon

MOBILITY += contacts versit    

HEADERS += ContactsTest.h SyncMLConfig.h

SOURCES += main.cpp \
	   ContactsTest.cpp \
	   ContactsStorage.cpp \
	   ContactsBackend.cpp \
       SimpleItem.cpp \
	   SyncMLConfig.cpp

LIBS += -L ../
LIBS += -lQtTest -lsynccommon -lsyncpluginmgr -lsyncprofile


QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
#install
testfiles.path = /usr/share/sync-app-tests/
testfiles.files =  hcontacts-tests.ref vcard1.txt vcard2.txt vcard3.txt

target.path = /usr/share/sync-app-tests/
INSTALLS += target \
            testfiles
