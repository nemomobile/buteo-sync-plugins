#include(doc/doc.pro)
TEMPLATE = subdirs

CONFIG += ordered \

SUBDIRS += syncmlcommon
SUBDIRS += clientplugins
SUBDIRS += storageplugins
SUBDIRS += storagechangenotifierplugins
SUBDIRS += doc

QT += QT_NO_EXCEPTIONS

testdefinition.path = /opt/tests/buteo-sync-plugins/test-definition
testdefinition.files = bin/tests.xml
tests.path = /opt/tests/buteo-sync-plugins/
tests.files =  \
              bin/runstarget.sh

INSTALLS += tests testdefinition
