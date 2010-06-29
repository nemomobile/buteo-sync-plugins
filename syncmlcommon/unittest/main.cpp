/*
 * This file is part of buteo-sync-plugins package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#include <QtTest/QtTest>

#include "ItemAdapterTest.h"
#include "SimpleItemTest.h"
#include "ItemIdMapperTest.h"
#include "SyncMLConfigTest.h"
#include "SyncMLStorageProviderTest.h"
#include "FolderItemParserTest.h"

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	ItemAdapterTest itemAdapterTest;
	SimpleItemTest simpleItemTest;
	ItemIdMapperTest mapperTest;
	SyncMLConfigTest configTest;
    Buteo::SyncMLStorageProviderTest storageTest;
	FolderItemParserTest parserTest;

	if (QTest::qExec(&simpleItemTest, argc, argv))
		return 1;
	if (QTest::qExec(&mapperTest, argc, argv))
		return 1;
	if (QTest::qExec(&itemAdapterTest, argc, argv))
		return 1;
	if (QTest::qExec(&configTest, argc, argv))
		return 1;        
        if (QTest::qExec(&storageTest, argc, argv))
                return 1;
	if (QTest::qExec(&parserTest, argc, argv))
		return 1;
	return 0;
}
