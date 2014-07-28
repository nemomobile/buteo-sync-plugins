#include(doc/doc.pro)
TEMPLATE = subdirs

syncmlcommon.subdir = syncmlcommon
syncmlcommon.target = sub-syncmlcommon

utils.subdir = utils
utils.target = sub-utils

clientplugins.subdir = clientplugins
clientplugins.target = sub-clientplugins
clientplugins.depends = sub-syncmlcommon

serverplugins.subdir = serverplugins
serverplugins.target = sub-serverplugins
serverplugins.depends = sub-syncmlcommon

storageplugins.subdir = storageplugins
storageplugins.target = sub-storageplugins
storageplugins.depends = sub-syncmlcommon

storagechangenotifierplugins.subdir = storagechangenotifierplugins
storagechangenotifierplugins.target = sub-storagechangenotifierplugins

doc.subdir = doc
doc.target = sub-doc

SUBDIRS += \
    syncmlcommon \
    utils \
    clientplugins \
    serverplugins \
    storageplugins \
    storagechangenotifierplugins \
    doc

testdefinition.path = /opt/tests/buteo-sync-plugins/test-definition
testdefinition.files = bin/tests.xml
tests.path = /opt/tests/buteo-sync-plugins/
tests.files = bin/runstarget.sh

INSTALLS += tests testdefinition

OTHER_FILES += rpm/*
