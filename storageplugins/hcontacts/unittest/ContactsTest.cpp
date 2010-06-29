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

#include "ContactsTest.h"
#include <QDebug>
#include <QFile>
#include <QtTest/QtTest>
#include <Logger.h>

static const QByteArray originalData(
    "BEGIN:VCARD\r\n"
    "VERSION:2.1\r\n"
    "REV:20091214T091343Z\r\n"
    "N:B;A\r\n"
    "SOUND:\r\n"
    "PHOTO:\r\n"
    "URL: \r\n"
    "TITLE:\r\n"
    "NOTE:\r\n"
    "END:VCARD\r\n");

/*
static const QByteArray originalData(
    "BEGIN:VCARD \r\n" \
    "VERSION:2.1 \r\n"
    "REV:20091106T102538Z \r\n"
    "N:Udupa;Kiran;Shama;Mr. \r\n"
    "FN:Udupa Kiran \r\n"
    "X-EPOCSECONDNAME: \r\n"
    "X-EPOCSECONDNAME: \r\n"
    "EMAIL;WORK:ext-kudupa2@nokia.com \r\n"
    "EMAIL;HOME:kudupa3@something.com \r\n"
    "X-SIP;SWIS: \r\n"
    "X-SIP;POC: \r\n"
    "X-SIP: \r\n"
    "X-SIP;HOME;VOIP: \r\n"
    "X-SIP;WORK;VOIP: \r\n"
    "X-SIP;VOIP: \r\n"
    "X-ASSISTANT: \r\n"
    "X-ASSISTANT: \r\n"
    "X-ANNIVERSARY: \r\n"
    "X-ANNIVERSARY: \r\n"
    "X-ASSISTANT-TEL: \r\n"
    "X-ASSISTANT-TEL: \r\n"
    "X-SPOUSE: \r\n"
    "X-SPOUSE: \r\n"
    "X-CHILDREN: \r\n"
    "X-CHILDREN:  \r\n"
    "NOTE:notes notes \r\n"
    "URL:www.web.com \r\n"
    "EMAIL:kudupa@nokia.com \r\n"
    "X-NICKNAME:KSU \r\n"
    "TITLE:Test Lead \r\n"
    "X-CHILDREN:kkkkk \r\n"
    "X-SPOUSE:xyz \r\n"
    "ORG:Wipro \r\n"
    "TEL:99999 \r\n"
    "TEL;CELL:44444 \r\n"
    "TEL;FAX;HOME:88888 \r\n"
    "TEL;FAX;WORK:77777 \r\n"
    "TEL;HOME;VOICE:33333 \r\n"
    "TEL;PAGER:66666 \r\n"
    "TEL;WORK;VOICE:11111 \r\n"
    "END:VCARD \r\n" );
*/

static const QByteArray modifiedData(
   "BEGIN:VCARD\r\n"
   "VERSION:2.1\r\n"
   "REV:20091214T091343Z\r\n"
   "N:B;A\r\n"
   "SOUND:\r\n"
   "PHOTO:\r\n"
   "URL: \r\n"
   "TITLE:\r\n"
   "NOTE:\r\n"
   "END:VCARD\r\n");

/*
static const QByteArray modifiedData(
    "BEGIN:VCARD \r\n" \
    "VERSION:2.1 \r\n"
    "REV:20091106T102538Z \r\n"
    "N:;Kiran;Shama;Mr. \r\n"
    "FN:Kiran \r\n"
    "X-EPOCSECONDNAME: \r\n"
    "X-EPOCSECONDNAME: \r\n"
    "EMAIL;WORK:kudupa2@wipro.com \r\n"
    "EMAIL;HOME:kudupa3@gmail.com \r\n"
    "X-SIP;SWIS: \r\n"
    "X-SIP;POC: \r\n"
    "X-SIP: \r\n"
    "X-SIP;HOME;VOIP: \r\n"
    "X-SIP;WORK;VOIP: \r\n"
    "X-SIP;VOIP: \r\n"
    "X-ASSISTANT: \r\n"
    "X-ASSISTANT: \r\n"
    "X-ANNIVERSARY: \r\n"
    "X-ANNIVERSARY: \r\n"
    "X-ASSISTANT-TEL: \r\n"
    "X-ASSISTANT-TEL: \r\n"
    "X-SPOUSE: \r\n"
    "X-SPOUSE: \r\n"
    "X-CHILDREN: \r\n"
    "X-CHILDREN:  \r\n"
    "NOTE:notes notes \r\n"
    "URL:www.web.com \r\n"
    "EMAIL:kudupa@nokia.com \r\n"
    "X-NICKNAME:KSU \r\n"
    "TITLE:Test Lead \r\n"
    "X-CHILDREN:kkkkk \r\n"
    "X-SPOUSE:xyz \r\n"
    "ORG:Wipro \r\n"
    "TEL:88888 \r\n"
    "TEL;CELL:44444 \r\n"
    "TEL;FAX;HOME:88888 \r\n"
    "TEL;FAX;WORK:77777 \r\n"
    "TEL;HOME;VOICE:33333 \r\n"
    "TEL;PAGER:66666 \r\n"
    "TEL;WORK;VOICE:11111 \r\n"
    "END:VCARD \r\n" );
*/

// allocate all resources needed here
void ContactsTest::initTestCase()
{
    Buteo::Logger::createInstance();

	iStorage = new ContactStorage("hcontacts");
    QVERIFY(iStorage != 0);

	QMap<QString, QString> props;
    QVERIFY(iStorage->init( props )) ;
}

// de-allocate all resources used
void ContactsTest::cleanupTestCase()
{
    QVERIFY(iStorage != 0);
    QVERIFY(iStorage->uninit());

    delete iStorage;
    iStorage = 0;

    Buteo::Logger::deleteInstance();
}

void ContactsTest::testSuite()
{
    runTestSuite( originalData, modifiedData, *iStorage );
}

void ContactsTest::runTestSuite( const QByteArray& aOriginalData, const QByteArray& aModifiedData,
                                 Buteo::StoragePlugin& aPlugin )
{
    const int WAIT_TIME = 2000;
    QByteArray data;

    qDebug() << "Starting suite...";

    // Get timestamp of 1 seconds in the past. Storages have timestamp accuracy of 1 seconds and
    // the current second is NOT included in getNewItems(), getModifiedItems() and getDeletedItems()
    QDateTime t1 = QDateTime::currentDateTime();
    qDebug() << "Marking time t1:" << t1;

    QTest::qSleep( WAIT_TIME );

    //  ** Test New Item
    Buteo::StorageItem* item = aPlugin.newItem();

    QVERIFY( item );
    QCOMPARE( item->getId(), QString( "" ) );

    //  ** Test Add Item

    QVERIFY( item->write( 0, aOriginalData ) );
    QVERIFY( item->getSize() == aOriginalData.size() );
    QVERIFY( item->read( 0, item->getSize(), data ) );
    QVERIFY( data == aOriginalData );

    qDebug() << "Adding new item...";
    Buteo::StoragePlugin::OperationStatus status = aPlugin.addItem( *item );

    QVERIFY( status == Buteo::StoragePlugin::STATUS_OK );
    QVERIFY( !item->getId().isEmpty() );

    QString id = item->getId();

    QTest::qSleep( WAIT_TIME );
    QDateTime t2 = QDateTime::currentDateTime();
    qDebug() << "Marking time t2:" << t2;
    QTest::qSleep( WAIT_TIME );

    // ** Check that item is now found from all items
    qDebug() << "Checking that the item is now found from getItems()...";

    QList<QString> items;
    QVERIFY( aPlugin.getAllItemIds( items ) );
    qDebug()  << "id expected" << id;
    QVERIFY( items.contains( id ) );

    // ** Check that item is now found from new items at t1
    qDebug() << "Checking that the item is found from getNewItems(t1)...";
    items.clear();
    QVERIFY( aPlugin.getNewItemIds( items, t1 ) );
    QVERIFY( items.contains( id ) );

    // ** Check that item is not found from new items at t2
    qDebug() << "Checking that the item is NOT found from getNewItems(t2)...";
    items.clear();
    QVERIFY( aPlugin.getNewItemIds( items, t2 ) );
    QVERIFY( !items.contains( id ) );

    //  ** Test Modify Item

    QVERIFY( item->write( 0, aModifiedData ) );
    QVERIFY( item->getSize() == aModifiedData.size() );
    QVERIFY( item->read( 0, item->getSize(), data ) );
    QVERIFY( data == aModifiedData );

    qDebug() << "Modifying item...";
    status = aPlugin.modifyItem( *item );
    QDateTime time = QDateTime::currentDateTime();
     qDebug() << "item modified at :" << time;

    QVERIFY( status == Buteo::StoragePlugin::STATUS_OK );
    QVERIFY( item->getId() == id );

    delete item;

    QTest::qSleep( WAIT_TIME );
    QDateTime t3 = QDateTime::currentDateTime();
    qDebug() << "Marking time t3:" << t3;
    QTest::qSleep( WAIT_TIME );

    // ** Check that item is still found from new items at t1
    qDebug() << "Checking that the item is found from getNewItems(t1)...";
    items.clear();
    QVERIFY( aPlugin.getNewItemIds( items, t1 ) );
    QVERIFY( items.contains( id ) );

    // ** Check that item is not found from modified items at t1
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t1)...";
    items.clear();
    QVERIFY( aPlugin.getModifiedItemIds( items, t1 ) );
    QVERIFY( !items.contains( id ) );

    // ** Check that item is now found from modified items at t2
    qDebug() << "Checking that the item is found from getModifiedItems(t2)...";
    items.clear();
    QVERIFY( aPlugin.getModifiedItemIds( items, t2 ) );
    QVERIFY( items.contains( id ) );

    // ** Check that item is not found from modified items at t3
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t3)...";
    items.clear();
    QVERIFY( aPlugin.getModifiedItemIds( items, t3 ) );
    QVERIFY( !items.contains( id ) );

    // ** Test Delete Item
    qDebug() << "Deleting item...";
    QVERIFY( aPlugin.deleteItem( id ) == Buteo::StoragePlugin::STATUS_OK );

    // ** Check that item is no longer found from new items at t1
    qDebug() << "Checking that the item is NOT found from getNewItems(t1)...";
    items.clear();
    QVERIFY( aPlugin.getNewItemIds( items, t1 ) );
    QVERIFY( !items.contains( id ) );

    // ** Check that item is not found from modified items at t1
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t1)...";
    items.clear();
    QVERIFY( aPlugin.getModifiedItemIds( items, t1 ) );
    QVERIFY( !items.contains( id ) );

    // commenting the below code as getDeletedItems is not supported by the backend.
    /*
    // ** Check that item is not found from deleted items at t1
    qDebug() << "Checking that the item is NOT found from getDeletedItems(t1)...";
    items.clear();
    QVERIFY( aPlugin.getDeletedItemIds( items, t1 ) );
    QVERIFY( !items.contains( id ) );
    */

    // ** Check that item is no longer found from modified items at t2
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t2)...";
    items.clear();
    QVERIFY( aPlugin.getModifiedItemIds( items, t2 ) );
    QVERIFY( !items.contains( id ) );

    // commenting the below code as getDeletedItems is not sypported by the backend
    /*
    // ** Check that item is now found from deleted items at t2
    qDebug() << "Checking that the item is now found from getDeletedItems(t2)...";
    items.clear();
    QVERIFY( aPlugin.getDeletedItemIds( items, t2 ) );
    VERIFY( items.contains( id ) );

    // ** Check that item is now found from deleted items at t3
    qDebug() << "Checking that the item is found from getDeletedItems(t3)...";
    items.clear();
    QVERIFY( aPlugin.getDeletedItemIds( items, t3 ) );
    QVERIFY( items.contains( id ) );
    */

    // ** Check that item is no longer found from all items
    qDebug() << "Checking that the item is NOT found from getItems()...";
    items.clear();
    QVERIFY( aPlugin.getAllItemIds( items ) );
    QVERIFY( !items.contains( id ) );
}


void ContactsTest::testBatchAddition()
{
    QList<Buteo::StorageItem*> itemsList;

    SimpleItem *item1 = new SimpleItem;
    item1->write(0, originalData);
    itemsList.append(item1);

    SimpleItem *item2 = new SimpleItem;
    item2->write(0, modifiedData);
    itemsList.append(item2);

    QList<Buteo::StoragePlugin::OperationStatus> statusList = iStorage->addItems(itemsList);
    QCOMPARE(statusList.size(), itemsList.size());
    QCOMPARE(statusList.first(), Buteo::StoragePlugin::STATUS_OK);
    QCOMPARE(statusList.last(), Buteo::StoragePlugin::STATUS_OK);

    delete item1;
    item1 = 0;

    delete item2;
    item2 = 0;
}


