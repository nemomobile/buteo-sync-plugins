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
#

TEMPLATE = app
TARGET = hnotes-tests
DEPENDPATH += . \
              ../ \

VPATH = .. \
    ../../../

INCLUDEPATH += . \
    ../ \
    ../../../syncmlcommon

LIBS += -L../../../syncmlcommon

HEADERS += NotesTest.h \
           NotesStorage.h \
           NotesBackend.h \
           syncmlcommon/SimpleItem.h \
           syncmlcommon/SyncMLConfig.h \
           syncmlcommon/SyncMLCommon.h


SOURCES += main.cpp \
           NotesTest.cpp \
           NotesStorage.cpp \
           NotesBackend.cpp \
           syncmlcommon/SimpleItem.cpp \
           syncmlcommon/SyncMLConfig.cpp

QT += testlib
QT -= gui
CONFIG += link_pkgconfig

equals(QT_MAJOR_VERSION, 4): {
    PKGCONFIG = buteosyncfw libkcalcoren libmkcal
    CONFIG += mkcal
    LIBS += -lsyncmlcommon
}

equals(QT_MAJOR_VERSION, 5): {
    PKGCONFIG = buteosyncfw5 libkcalcoren-qt5 libmkcal-qt5
    LIBS += -lsyncmlcommon5
}

QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage

#install
tests.files = hnotes-tests.ref
tests.path = /opt/tests/buteo-sync-plugins/
target.path = /opt/tests/buteo-sync-plugins/
INSTALLS += target \
            tests
