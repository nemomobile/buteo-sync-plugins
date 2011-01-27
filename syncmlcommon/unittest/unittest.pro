#/*
# * This file is part of buteo-sync-plugins package
# *
# * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
# *
# * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
# *
# * This library is free software; you can redistribute it and/or
# * modify it under the terms of the GNU Lesser General Public License
# * version 2.1 as published by the Free Software Foundation.
# *
# * This library is distributed in the hope that it will be useful, but
# * WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# * Lesser General Public License for more details.
# *
# * You should have received a copy of the GNU Lesser General Public
# * License along with this library; if not, write to the Free Software
# * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# * 02110-1301 USA
# *
# */

TEMPLATE = app
TARGET = syncmlcommon-tests
DEPENDPATH += . \
              ../ \

INCLUDEPATH += . \
               ../ \
               /usr/include/libsynccommon

LIBS += -lsyncpluginmgr -lsynccommon -lmeegosyncml -lsyncprofile -lQtTest

DEFINES +=  SYNC_APP_UNITTESTS

#inputs
HEADERS += ItemAdapterTest.h \
           ../ItemAdapter.h \
           ../SimpleItem.h \
           SimpleItemTest.h \
           ../ItemIdMapper.h \
           ItemIdMapperTest.h \
           ../SyncMLConfig.h \
           SyncMLConfigTest.h \
           ../StorageAdapter.h \
           ../SyncMLStorageProvider.h \
           SyncMLStorageProviderTest.h \
	       FolderItemParserTest.h \
           ../FolderItemParser.h \
           DeviceInfoTest.h \
           ../DeviceInfo.h \
           

SOURCES += main.cpp \
           ItemAdapterTest.cpp \
           ../ItemAdapter.cpp \
           ../SimpleItem.cpp \
           SimpleItemTest.cpp \
           ../ItemIdMapper.cpp \
           ItemIdMapperTest.cpp \
           ../SyncMLConfig.cpp \
           SyncMLConfigTest.cpp \
           ../StorageAdapter.cpp \
           ../SyncMLStorageProvider.cpp \
           SyncMLStorageProviderTest.cpp \
	       FolderItemParserTest.cpp \
           ../FolderItemParser.cpp \
           DeviceInfoTest.cpp \
           ../DeviceInfo.cpp 
           

QT += testlib sql xml
QT -= gui
CONFIG += debug qtestlib

QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage

#install
tests.files = syncmlcommon-tests.ref
tests.path = /usr/share/sync-app-tests/
target.path = /usr/share/sync-app-tests/
INSTALLS += target \
            tests
