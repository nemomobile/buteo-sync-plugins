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
#include <QByteArray>

#include "ItemAdapterTest.h"
#include "SimpleItem.h"

void ItemAdapterTest::initTestCase()
{
	iItemAd = new ItemAdapter(0);
	QCOMPARE(iItemAd->isValid(), false);
	Buteo::StorageItem *storage = new SimpleItem();
	iItemAd = new ItemAdapter(storage);
	QCOMPARE(iItemAd->isValid(), true);
}
void ItemAdapterTest::cleanupTestCase()
{
	QVERIFY(iItemAd);
	delete iItemAd;
	
	iItemAd = 0;
}
void ItemAdapterTest::testReadWriteSize()
{
	QByteArray byte("ItemAdapter");
	QByteArray byte1;
	QCOMPARE(iItemAd->write(4, byte), true);
	QCOMPARE(iItemAd->getSize(),(qint64)(byte.size()+4));
	QCOMPARE(iItemAd->read(4, -1, byte1), true);
	QVERIFY(byte1.contains("ItemAdapter"));
}
