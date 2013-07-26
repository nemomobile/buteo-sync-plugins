#include(doc/doc.pro)
TEMPLATE = subdirs

CONFIG += ordered \

SUBDIRS += syncmlcommon \
    utils \
    clientplugins \
    storageplugins \
    storagechangenotifierplugins \
    doc \
    serverplugins \
    serverplugins/syncmlserver

QT += QT_NO_EXCEPTIONS

testdefinition.path = /opt/tests/buteo-sync-plugins/test-definition
testdefinition.files = bin/tests.xml
tests.path = /opt/tests/buteo-sync-plugins/
tests.files =  \
              bin/runstarget.sh

INSTALLS += tests testdefinition
