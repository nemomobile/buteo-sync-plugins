TEMPLATE = app
TARGET = hcontacts-tests

QT += core testlib sql
QT -= gui
CONFIG += link_pkgconfig testlib

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    PKGCONFIG = buteosyncfw buteosyncml qtcontacts-sqlite-extensions
    MOBILITY += contacts versit
    LIBS += -lsyncmlcommon -lQtTest
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG = buteosyncfw5 Qt5Contacts Qt5Versit buteosyncml5 qtcontacts-sqlite-qt5-extensions
    LIBS += -lsyncmlcommon5 -lQt5Test
}

DEPENDPATH += . \
              ../ \
              ../../../syncmlcommon

INCLUDEPATH += . \
    ../ \
    ../../../syncmlcommon

HEADERS += ContactsTest.h

SOURCES += main.cpp \
       ContactsTest.cpp

LIBPATH += ../ ../../../syncmlcommon
LIBS += -lhcontacts-storage

QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
#install
testfiles.path = /opt/tests/buteo-sync-plugins/
testfiles.files =  hcontacts-tests.ref vcard1.txt vcard2.txt vcard3.txt

target.path = /opt/tests/buteo-sync-plugins/
INSTALLS += target \
            testfiles
