/*
 * This file is part of buteo-sync-plugins package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2014 Jolla Ltd.
 *
 * Contributors: Sateesh Kavuri <sateesh.kavuri@nokia.com>
 *               Valério Valério <valerio.valerio@jolla.com>
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

#include <QVersitReader>
#include <QVersitContactImporter>
#include <QtTest/QtTest>
#include <QDebug>
#include <LogMacros.h>
#include <Logger.h>

#include "ContactsStorage.h"
#include "SimpleItem.h"

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

// allocate all resources needed here
void ContactsTest::initTestCase()
{
    Buteo::Logger::createInstance();
}

// de-allocate all resources used
void ContactsTest::cleanupTestCase()
{
    Buteo::Logger::deleteInstance();
}

void ContactsTest::testSuiteSingle()
{
    QMap<QString, QString> props;

    ContactStorage storage("hcontacts");
    QVERIFY(storage.init( props )) ;

    runTestSuite( originalData, modifiedData, storage, false );

    QVERIFY(storage.uninit());
}

void ContactsTest::testSuiteBatched()
{
    QMap<QString, QString> props;

    ContactStorage storage("hcontacts");
    QVERIFY(storage.init( props )) ;

    runTestSuite( originalData, modifiedData, storage, true );

    QVERIFY(storage.uninit());
}

void ContactsTest::runTestSuite( const QByteArray& aOriginalData, const QByteArray& aModifiedData,
                                 Buteo::StoragePlugin& aPlugin, bool aBatched )
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
    Buteo::StoragePlugin::OperationStatus status;
    if( aBatched )
    {
        QList<Buteo::StorageItem*> items;
        items.append( item );

        QList<Buteo::StoragePlugin::OperationStatus> results = aPlugin.addItems( items);
        QCOMPARE( results.count(), 1 );
        status = results.first();
    }
    else
    {
        status = aPlugin.addItem( *item );
    }

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
    if( aBatched )
    {
        QList<Buteo::StorageItem*> items;
        items.append( item );

        QList<Buteo::StoragePlugin::OperationStatus> results = aPlugin.modifyItems( items);
        QCOMPARE( results.count(), 1 );
        status = results.first();
    }
    else
    {
        status = aPlugin.modifyItem( *item );
    }




    QDateTime time = QDateTime::currentDateTime();
    qDebug() << "Item modified at :" << time;

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
    if( aBatched )
    {
        QList<QString> items;
        items.append( id );

        QList<Buteo::StoragePlugin::OperationStatus> results = aPlugin.deleteItems( items );
        QCOMPARE( results.count(), 1 );
        status = results.first();
    }
    else
    {
        status = aPlugin.deleteItem( id );
    }

    QVERIFY( status == Buteo::StoragePlugin::STATUS_OK );

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

    // ** Check that item is not found from deleted items at t1
    qDebug() << "Checking that the item is NOT found from getDeletedItems(t1)...";
    items.clear();
    QVERIFY( aPlugin.getDeletedItemIds( items, t1 ) );
    QVERIFY( !items.contains( id ) );

    // ** Check that item is no longer found from modified items at t2
    qDebug() << "Checking that the item is NOT found from getModifiedItems(t2)...";
    items.clear();
    QVERIFY( aPlugin.getModifiedItemIds( items, t2 ) );
    QVERIFY( !items.contains( id ) );

    // ** Check that item is now found from deleted items at t2
    qDebug() << "Checking that the item is now found from getDeletedItems(t2)...";
    items.clear();
    QVERIFY( aPlugin.getDeletedItemIds( items, t2 ) );
    QVERIFY( items.contains( id ) );

    // ** Check that item is now found from deleted items at t3
    qDebug() << "Checking that the item is found from getDeletedItems(t3)...";
    items.clear();
    QVERIFY( aPlugin.getDeletedItemIds( items, t3 ) );
    QVERIFY( items.contains( id ) );
    // ** Check that item is no longer found from all items
    qDebug() << "Checking that the item is NOT found from getItems()...";
    items.clear();
    QVERIFY( aPlugin.getAllItemIds( items ) );
    QVERIFY( !items.contains( id ) );
}

/*
void ContactsTest::pf177715()
{
    const QString vcardTemplate(
        "BEGIN:VCARD\r\n"
        "VERSION:2.1\r\n"
        "N:%1;%2\r\n"
        "TEL:%3\r\n"
        "END:VCARD\r\n");

    const int numContacts = 1000;
    const int contactsPerBatch = 20;

    QStringList vcards;

    for( int i = 0; i < numContacts; ++i )
    {

        QString number = QString::number(i);
        QString vcard = vcardTemplate.arg( number ).arg( number ).arg( number);
        vcards.append( vcard);
    }

    qDebug() << "Testing with" << numContacts << "contacts, using batches of" << contactsPerBatch;
    QTime timer;

    QStringList availableManagers = QContactManager::availableManagers();
    QContactManager manager( "tracker" );



    qDebug() << "Clearing existing contacts...";
    QList<QContactLocalId> contactIDs = manager.contactIds();
    for( int i = 0; i < contactIDs.count(); i += contactsPerBatch )
    {
        directDelete( contactIDs.mid( i, contactsPerBatch ), manager );
    }

    qDebug() << "Starting direct add...";
    timer.start();

    for( int i = 0; i < numContacts; i += contactsPerBatch )
    {
        directAdd( vcards.mid( i, contactsPerBatch ), manager );
    }

    qDebug() << "Direct add finished, took" << timer.elapsed() << "ms";



    qDebug() << "Starting direct ids...";
    timer.start();

    contactIDs = manager.contactIds();

    qDebug() << "Get  direct ids finished, took" << timer.elapsed() << "ms";



    qDebug() << "Starting direct delete...";
    timer.restart();

    contactIDs = manager.contactIds();
    for( int i = 0; i < contactIDs.count(); i += contactsPerBatch )
    {
        directDelete( contactIDs.mid( i, contactsPerBatch ), manager );
    }
    qDebug() << "Direct delete finished, took" << timer.elapsed() << "ms";

    qDebug() << "Initiating plugin...";
    timer.restart();

    QMap<QString, QString> props;
    ContactStorage storage("hcontacts");
    QVERIFY(storage.init( props )) ;

    qDebug() << "Plugin init finished, took" << timer.elapsed() << "ms";



    QList<Buteo::StorageItem*> items;
    for( int i = 0; i < numContacts; ++i )
    {
        Buteo::StorageItem* item = storage.newItem();
        QVERIFY( item );
        QVERIFY( item->write( 0, vcards[i].toAscii() ) );
        items.append( item );
    }



    qDebug() << "Starting plugin add...";
    timer.start();

    for( int i = 0; i < numContacts; i += contactsPerBatch )
    {
        QList<Buteo::StoragePlugin::OperationStatus> status = storage.addItems( items.mid(i, contactsPerBatch ) );
        QVERIFY( status.contains( Buteo::StoragePlugin::STATUS_OK ) == contactsPerBatch );
    }

    qDebug() << "Plugin add finished, took" << timer.elapsed() << "ms";



    qDeleteAll( items );
    items.clear();


    qDebug() << "Starting plugin ids...";
    timer.start();

    QList<QString> ids;

    QVERIFY( storage.getAllItemIds( ids) );

    qDebug() << "Get  plugin ids finished, took" << timer.elapsed() << "ms";

    qDebug() << "Starting plugin delete...";
    timer.restart();

    contactIDs = manager.contactIds();
    for( int i = 0; i < contactIDs.count(); i += contactsPerBatch )
    {
        QList<Buteo::StoragePlugin::OperationStatus> status = storage.deleteItems( ids.mid( i, contactsPerBatch ) );
                QVERIFY( status.contains( Buteo::StoragePlugin::STATUS_OK ) == contactsPerBatch );
    }
    qDebug() << "Plugin delete finished, took" << timer.elapsed() << "ms";


    qDebug() << "Uninitiating plugin...";
    timer.restart();

    QVERIFY(storage.uninit()) ;

    qDebug() << "Plugin uninit finished, took" << timer.elapsed() << "ms";

}

void ContactsTest::directAdd( const QStringList& aVCards, QContactManager& aManager )
{

    QByteArray byteArray;
    //QVersitReader needs LF/CRLF/CR between successive vcard's in the list,
    //CRLF didn't work though.
    QString LF = "\n";

    foreach ( QString vcard, aVCards )
    {
        byteArray.append(vcard.toUtf8());
        byteArray.append(LF.toUtf8());
    }

    QBuffer readBuf(&byteArray);
    readBuf.open(QIODevice::ReadOnly);
    readBuf.seek(0);

    QVersitReader versitReader;
    versitReader.setDevice (&readBuf);

    if (!versitReader.startReading())
            LOG_WARNING ("Error while reading vcard");

    if (!versitReader.waitForFinished())
            LOG_WARNING ("Error while finishing reading vcard");

    QList<QVersitDocument> versitDocList = versitReader.results ();
    readBuf.close();

    QVersitContactImporter contactImporter;
    QList<QContact> contactList;
    QVERIFY( contactImporter.importDocuments(versitDocList) );
    contactList =  contactImporter.contacts();

    QMap<int, QContactManager::Error>  errorMap;

    QVERIFY( aManager.saveContacts(&contactList, &errorMap) );
    QVERIFY( errorMap.isEmpty() );

}

void ContactsTest::directDelete( const QList<QContactLocalId>& aContactIds, QContactManager& manager )
{

    QMap<int , QContactManager::Error> errorMap;

    QVERIFY( manager.removeContacts( aContactIds, &errorMap ) );
    QVERIFY( errorMap.isEmpty() );
}

*/

QTEST_MAIN(ContactsTest)

