TEMPLATE = app
TARGET = hcontacts-tests

QT += core testlib sql gui
CONFIG += link_pkgconfig

equals(QT_MAJOR_VERSION, 4): {
    CONFIG += mobility
    PKGCONFIG = buteosyncfw buteosyncml qtcontacts-sqlite-extensions
    MOBILITY += contacts versit
    LIBS += -lsyncmlcommon
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG = buteosyncfw5 Qt5Contacts Qt5Versit buteosyncml5 qtcontacts-sqlite-qt5-extensions
    LIBS += -lsyncmlcommon5
}

DEPENDPATH += . \
              ../ \

VPATH = .. \
    ../../../

INCLUDEPATH += . \
    ../ \
    ../../../syncmlcommon

LIBS += -L../../../syncmlcommon

HEADERS += ContactsTest.h \
           ContactsStorage.h \
           ContactsBackend.h \
           ContactsImport.h \
           ContactPropertyHandler.h

SOURCES += ContactsTest.cpp \
           ContactsStorage.cpp \
           ContactsBackend.cpp \
           ContactsImport.cpp \
           ContactPropertyHandler.cpp

QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
#install
testfiles.path = /opt/tests/buteo-sync-plugins/
testfiles.files =  hcontacts-tests.ref vcard1.txt vcard2.txt vcard3.txt

target.path = /opt/tests/buteo-sync-plugins/
INSTALLS += target \
            testfiles
