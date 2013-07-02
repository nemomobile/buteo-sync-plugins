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

#include "CalendarTest.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <buteosyncfw5/StorageItem.h>
#else
#include <buteosyncfw/StorageItem.h>
#endif

#include <QtTest>

CalendarTest::CalendarTest() : iCalendarStorage( "hcalendar" )
{

}
CalendarTest::~CalendarTest()
{

}

void CalendarTest::initTestCase()
{

    QMap<QString, QString> props;
    props[NOTEBOOKNAME] = "testnotebook";

    QVERIFY( iCalendarStorage.init( props ) );
}

void CalendarTest::cleanupTestCase()
{
    QVERIFY( iCalendarStorage.uninit() );

}

void CalendarTest::testSuite()
{

    const QByteArray originalData( "BEGIN:VCALENDAR\r\n" \
                                   "VERSION:1.0\r\n" \
                                   "BEGIN:VEVENT\r\n" \
                                   "UID:D5G8OBE64EFnl46ib91EM2\r\n" \
                                   "SUMMARY:Test\r\n" \
                                   "DTSTART:20090909T080000\r\n" \
                                   "DTEND:20090909T080000\r\n" \
                                   "CLASS:PRIVATE\r\n" \
                                   "LOCATION:Hell\r\n" \
                                   "PRIORITY:2\r\n" \
                                   "END:VEVENT\r\n" \
                                   "END:VCALENDAR\r\n" );

    const QByteArray modifiedData( "BEGIN:VCALENDAR\r\n" \
                                   "VERSION:1.0\r\n" \
                                   "BEGIN:VEVENT\r\n" \
                                   "UID:D5G8OBE64EFnl46ib9\r\n" \
                                   "SUMMARY:Test2\r\n" \
                                   "DTSTART:20090909T090000\r\n" \
                                   "DTEND:20090909T090000\r\n" \
                                   "CLASS:PRIVATE\r\n" \
                                   "LOCATION:Heaven\r\n" \
                                   "PRIORITY:2\r\n" \
                                   "END:VEVENT\r\n" \
                                   "END:VCALENDAR\r\n" );

    runTestSuite( originalData, modifiedData, iCalendarStorage );

}

void CalendarTest::runTestSuite( const QByteArray& aOriginalData, const QByteArray& aModifiedData,
                                 Buteo::StoragePlugin& aPlugin )
{
    QByteArray data;

    qDebug() << "Starting suite...";

    // Get timestamp of 1 seconds in the past. Storages have timestamp accuracy of 1 seconds and
    // the current second is NOT included in getNewItems(), getModifiedItems() and getDeletedItems()
    QDateTime t1 = QDateTime::currentDateTime();
    qDebug() << "Marking time t1:" << t1;

    QTest::qSleep( 2000 );

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

    QTest::qSleep( 2000 );
    QDateTime t2 = QDateTime::currentDateTime();
    qDebug() << "Marking time t2:" << t2;
    QTest::qSleep( 2000 );


    // ** Check that item is now found from all items
    qDebug() << "Checking that the item is now found from getItems()...";

    QList<QString> items;
    QVERIFY( aPlugin.getAllItemIds( items ) );

    bool found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    QVERIFY( found );

    items.clear();

    // ** Check that item is now found from new items at t1
    qDebug() << "Checking that the item is found from getNewItems(t1)...";
    QVERIFY( aPlugin.getNewItemIds( items, t1 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( found );

    // ** Check that item is not found from new items at t2
    qDebug() << "Checking that the item is NOT found from getNewItems(t2)...";

    // @todo: There's no mechanism in storage plugin API for error handling,
    //        therefore we can't check if this call failed or not
    QVERIFY( aPlugin.getNewItemIds( items, t2 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( !found );

    //  ** Test Modify Item

    QVERIFY( item->write( 0, aModifiedData ) );
    QVERIFY( item->getSize() == aModifiedData.size() );
    QVERIFY( item->read( 0, item->getSize(), data ) );
    QVERIFY( data == aModifiedData );

    qDebug() << "Modifying item...";
    status = aPlugin.modifyItem( *item );

    QVERIFY( status == Buteo::StoragePlugin::STATUS_OK );
    QVERIFY( item->getId() == id );

    delete item;

    QTest::qSleep( 2000 );
    QDateTime t3 = QDateTime::currentDateTime();
    qDebug() << "Marking time t3:" << t3;
    QTest::qSleep( 2000 );

    // ** Check that item is still found from new items at t1
    qDebug() << "Checking that the item is found from getNewItems(t1)...";
    QVERIFY( aPlugin.getNewItemIds( items, t1 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( found );

    // ** Check that item is not found from modified items at t1
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t1)...";
    QVERIFY( aPlugin.getModifiedItemIds( items, t1 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( !found );

    // ** Check that item is now found from modified items at t2
    qDebug() << "Checking that the item is found from getModifiedItems(t2)...";
    QVERIFY( aPlugin.getModifiedItemIds( items, t2 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( found );

    // ** Check that item is not found from modified items at t3
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t3)...";
    QVERIFY( aPlugin.getModifiedItemIds( items, t3 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( !found );

    // ** Test Delete Item
    qDebug() << "Deleting item...";
    QVERIFY( aPlugin.deleteItem( id ) == Buteo::StoragePlugin::STATUS_OK );

    // ** Check that item is no longer found from new items at t1
    qDebug() << "Checking that the item is NOT found from getNewItems(t1)...";
    QVERIFY( aPlugin.getNewItemIds( items, t1 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( !found );


    // ** Check that item is not found from modified items at t1
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t1)...";
    QVERIFY( aPlugin.getModifiedItemIds( items, t1 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( !found );


    // ** Check that item is not found from deleted items at t1
    qDebug() << "Checking that the item is NOT found from getDeletedItems(t1)...";
    QVERIFY( aPlugin.getDeletedItemIds( items, t1 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( !found );

    // ** Check that item is no longer found from modified items at t2
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t2)...";
    QVERIFY( aPlugin.getModifiedItemIds( items, t2 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( !found );


    // ** Check that item is now found from deleted items at t2
    qDebug() << "Checking that the item is now found from getDeletedItems(t2)...";
    QVERIFY( aPlugin.getDeletedItemIds( items, t2 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( found );

    // ** Check that item is now found from deleted items at t3
    qDebug() << "Checking that the item is found from getDeletedItems(t3)...";
    QVERIFY( aPlugin.getDeletedItemIds( items, t3 ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( found );

    // ** Check that item is no longer found from all items
    qDebug() << "Checking that the item is NOT found from getItems()...";
    QVERIFY( aPlugin.getAllItemIds( items ) );

    found = false;
    for( int i = 0; i < items.count(); ++i ) {

        if( items[i] == id ) {
            found = true;
            break;
        }

    }

    items.clear();

    QVERIFY( !found );
}
