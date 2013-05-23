TEMPLATE = app
TARGET = hcontacts-tests 
 
QT += core testlib sql
CONFIG += link_pkgconfig qtestlib mobility
PKGCONFIG = buteosyncfw

DEPENDPATH += . \
              ../ \
              ../../../syncmlcommon

INCLUDEPATH += . \
    ../ \
    ../../../syncmlcommon

MOBILITY += contacts versit    

HEADERS += ContactsTest.h SyncMLConfig.h

SOURCES += main.cpp \
	   ContactsTest.cpp \
	   ContactsStorage.cpp \
	   ContactsBackend.cpp \
       SimpleItem.cpp \
           SyncMLConfig.cpp \
           ContactDetailHandler.cpp

LIBS += -L ../
LIBS += -lQtTest


QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
#install
testfiles.path = /opt/tests/buteo-sync-plugins/
testfiles.files =  hcontacts-tests.ref vcard1.txt vcard2.txt vcard3.txt

target.path = /opt/tests/buteo-sync-plugins/
INSTALLS += target \
            testfiles
