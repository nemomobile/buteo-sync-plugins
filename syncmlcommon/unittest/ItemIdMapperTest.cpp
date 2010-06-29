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
#include "ItemIdMapperTest.h"
#include "Logger.h"

void ItemIdMapperTest::initTestCase()
{
	iMapper = new ItemIdMapper();
	QCOMPARE(iMapper->iRecursionGuard, false);
	//Logger::createInstance();
}

void ItemIdMapperTest::cleanupTestCase()
{
	QVERIFY(iMapper);
	delete iMapper;
	
	iMapper = 0;
	//Logger::deleteInstance();
}

void ItemIdMapperTest::testInit()
{
	QString DbFile; //= "/scratchbox/users/vineela/home/vineela/database.db";
	QCOMPARE(iMapper->iDb.isOpen(), false);
	QCOMPARE(iMapper->init("database.db", "plugin"), true);
	QCOMPARE(iMapper->iDb.isOpen(), true);
	QCOMPARE(iMapper->iDb.isValid(), true);
    QCOMPARE(iMapper->iDb.connectionName(), QString("idmapper0"));
    QCOMPARE(iMapper->iDb.databaseName(), QString("database.db"));
    QCOMPARE(iMapper->iStorageId, QString("plugin"));
}

void ItemIdMapperTest::testKeyValueAdd()
{
	//testing the function add(const QString& aKey)
    QCOMPARE(iMapper->iStorageId, QString("plugin"));
    QCOMPARE(iMapper->add("key"), QString("1"));
	QCOMPARE(iMapper->iRecursionGuard, false);
	
	//testing the function key(const QString& aValue)
	QString val = "";
	QCOMPARE(val.isEmpty(), true);
	QCOMPARE(iMapper->key(val), val);
    QCOMPARE(iMapper->key("key"), QString("key"));
	
	//testing the function value(const QString& aValue)
	QCOMPARE(val.isEmpty(), true);
	QCOMPARE(iMapper->value(val), val);
    QCOMPARE(iMapper->value("key"), QString("1"));

    // Mapping id that is already an integer does nothing.
    QCOMPARE(iMapper->value("1234"), QString("1234"));
    QCOMPARE(iMapper->key("1234"), QString("1234"));
}

void ItemIdMapperTest::testUninit()
{
	QStringList list = iMapper->iDb.tables(QSql::Tables);
	QVERIFY(list.contains("plugin"));
	
	iMapper->uninit();
	QCOMPARE(iMapper->iDb.tables(QSql::Tables).isEmpty(), true);
	QCOMPARE(iMapper->iDb.isOpen(), false);
}
