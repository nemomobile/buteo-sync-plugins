#include(doc/doc.pro)
TEMPLATE = subdirs

CONFIG += ordered \

SUBDIRS += syncmlcommon
SUBDIRS += clientplugins
SUBDIRS += storageplugins
SUBDIRS += storagechangenotifierplugins
SUBDIRS += doc

QT += QT_NO_EXCEPTIONS

tests.path = /usr/share/sync-app-tests/
tests.files = bin/tests.xml \
              bin/runstarget.sh

INSTALLS += tests
