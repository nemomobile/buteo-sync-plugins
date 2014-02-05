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
#include <QFile>
#include <QStringListIterator>
#include <LogMacros.h>
#include "ContactsStorage.h"
#include "SimpleItem.h"
#include "SyncMLCommon.h"
#include "SyncMLConfig.h"
#include "ProfileEngineDefs.h"

#include <QContact>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
QTM_USE_NAMESPACE
#endif

const char* CTCAPSFILENAME11 = "CTCaps_contacts_11.xml";
const char* CTCAPSFILENAME12 = "CTCaps_contacts_12.xml";


extern "C" Buteo::StoragePlugin*  createPlugin(const QString& aPluginName)
{
    return new ContactStorage(aPluginName);
}

extern "C" void destroyPlugin(Buteo::StoragePlugin* storage)
{
    delete storage;
}

ContactStorage::ContactStorage(const QString& aPluginName)
 : Buteo::StoragePlugin(aPluginName), iBackend( 0 )
{
    FUNCTION_CALL_TRACE;
}

ContactStorage::~ContactStorage()
{
    FUNCTION_CALL_TRACE;
    if(iBackend) {
        LOG_WARNING( "Uninit method has not been called!" );
        delete iBackend;
        iBackend = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////       Below functions are derived from storage plugin   ///////////////
////////////////////////////////////////////////////////////////////////////////////

bool ContactStorage::init( const QMap<QString, QString>& aProperties )
{
    FUNCTION_CALL_TRACE;

    iDeletedItems.uninit();

    const QString dbFile = "hcontacts.db";
    QString fullDbPath = SyncMLConfig::getDatabasePath() + dbFile;

    if( !iDeletedItems.init(fullDbPath) ) {
        return false;
    }

    QVersitDocument::VersitType vCardVersion;

    iProperties = aProperties;

    QString propVersion = iProperties[STORAGE_DEFAULT_MIME_VERSION_PROP];

    if(propVersion == "3.0") {
        LOG_DEBUG( "Storage is using VCard version 3.0" );
        vCardVersion =  QVersitDocument::VCard30Type;
        iProperties[STORAGE_DEFAULT_MIME_PROP] = "text/vcard";
    }
    else {
        LOG_DEBUG( "Storage is using VCard version 2.1");
        vCardVersion = QVersitDocument::VCard21Type;
    }

    iProperties[STORAGE_SYNCML_CTCAPS_PROP_11] = getCtCaps( CTCAPSFILENAME11 );
    iProperties[STORAGE_SYNCML_CTCAPS_PROP_12] = getCtCaps( CTCAPSFILENAME12 );

    iBackend = new ContactsBackend(vCardVersion,
                                   iProperties.value(Buteo::KEY_STORAGE_SYNC_TARGET),
                                   iProperties.value(Buteo::KEY_STORAGE_ORIGIN_ID));

    if( !iBackend->init() ) {
        LOG_CRITICAL( "Failed to init contacts backend" );
        return false;
    }

    if( !doInitItemAnalysis() ) {
        return false;
    }

    return true;
}

bool ContactStorage::uninit()
{
    FUNCTION_CALL_TRACE;

    doUninitItemAnalysis();

    // If the backend object is NULL, there is nothing to do anyway,
    // so the default value can be 'true' here.
    bool backendUninitOk = true;

    if( iBackend != NULL) {
        backendUninitOk = iBackend->uninit();
        delete iBackend;
        iBackend = NULL;
    }

    bool deleteItemsIdStorageUninitOk = iDeletedItems.uninit();

    return (backendUninitOk && deleteItemsIdStorageUninitOk);
}

bool ContactStorage::getAllItems(QList<Buteo::StorageItem*> &aItems)
{
        FUNCTION_CALL_TRACE;
        bool operationStatus = false;
        QList<QContactLocalId>  list;

        if(iBackend) {
                list = iBackend->getAllContactIds();
                if(list.size() != 0) {
                        qDebug()  << " Number of items retrieved from Contacts " << list.size();
                        aItems = getStoreList(list);
                }
                operationStatus = true;
        }
        return operationStatus;
}

bool ContactStorage::getAllItemIds( QList<QString>& aItems )
{
        FUNCTION_CALL_TRACE;
        bool operationStatus = false;
        QList<QContactLocalId>  list;
        if(iBackend) {
                list = iBackend->getAllContactIds();
                qDebug() << " Number of items retrieved from Contacts " << list.size();
                foreach(QContactLocalId id , list) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                        aItems.append(id.toString());
#else
                        aItems.append(QString::number(id));
#endif
                }
                operationStatus = true;
        }
        return operationStatus;
}

bool ContactStorage::getNewItems(QList<Buteo::StorageItem*> &aItems ,const QDateTime& aTime)
{
        FUNCTION_CALL_TRACE;
        bool operationStatus = false;
        QList<QContactLocalId>  list;
        if(iBackend) {
                qDebug()  << "****** getNewItems : Added After: ********" << aTime;
                list = iBackend->getAllNewContactIds(aTime);
                if(list.size() != 0) {
                        qDebug()  << "New Item List Size is " << list.size();
                        aItems = getStoreList(list);
                }
                operationStatus = true;
        }
        return operationStatus;
}

bool ContactStorage::getNewItemIds( QList<QString>& aNewItemIds, const QDateTime& aTime )
{
        FUNCTION_CALL_TRACE;
        bool operationStatus = false;
        QList<QContactLocalId>  list;

        if(iBackend) {
                qDebug()  << "****** getNewItem Ids : Added After: ********" << aTime;
                list = iBackend->getAllNewContactIds(aTime);

                foreach(QContactLocalId id , list) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                        aNewItemIds.append(id.toString());
#else
                        aNewItemIds.append(QString::number(id));
#endif
                }

                operationStatus = true;
        }
        return operationStatus;
}

bool ContactStorage::getModifiedItems( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime )
{
        FUNCTION_CALL_TRACE;
        QList<QContactLocalId>  list;
        bool operationStatus = false;

        if(iBackend) {
                qDebug() << "******* getModifiedItems: From ********" << aTime;

                list = iBackend->getAllModifiedContactIds(aTime);

                aModifiedItems = getStoreList(list);

                operationStatus = true;
        }

        return operationStatus;
}

bool ContactStorage::getModifiedItemIds( QList<QString>& aModifiedItemIds, const QDateTime& aTime )
{
        FUNCTION_CALL_TRACE;
        QList<QContactLocalId>  list;
        bool operationStatus = false;

        if(iBackend) {
                qDebug() << "******* getModifiedItemIds : From ********" << aTime;

                list = iBackend->getAllModifiedContactIds(aTime);

                foreach(QContactLocalId id , list) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                        aModifiedItemIds.append(id.toString());
#else
                        aModifiedItemIds.append(QString::number(id));
#endif
                }
                operationStatus = true;
        }
        return operationStatus;
}

bool ContactStorage::getDeletedItemIds( QList<QString>& aDeletedItemIds, const QDateTime& aTime )
{
        FUNCTION_CALL_TRACE;
    LOG_DEBUG( "Getting deleted contacts since" << aTime );

    return iDeletedItems.getDeletedItems( aDeletedItemIds, aTime );
}

Buteo::StorageItem* ContactStorage::newItem()
{
    FUNCTION_CALL_TRACE;
    return new SimpleItem;
}

QList<Buteo::StorageItem*> ContactStorage::getItems( const QStringList& aItemIdList )
{
    FUNCTION_CALL_TRACE;

    QList<Buteo::StorageItem*> items;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QMap<QString,QString> vcards;
#else
    QMap<QContactLocalId,QString> vcards;
#endif
    QList<QContactLocalId> ids;
    QStringListIterator itr( aItemIdList );

    if( iBackend )
    {
        foreach (const QString itr, aItemIdList)
        {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            ids.append( QContactId::fromString (itr) );
#else
            ids.append( itr.toUInt() );
#endif
        }
        iBackend->getContacts( ids, vcards );
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QMapIterator<QString,QString> i(vcards);
#else
        QMapIterator<QContactLocalId,QString> i(vcards);
#endif
        while( i.hasNext() )
        {
            i.next();
            if( !i.value().isEmpty() )
            {
                SimpleItem *item = new SimpleItem;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                item->setId( i.key() );
#else
                item->setId( QString::number( i.key() ) );
#endif
                item->setType( iProperties[STORAGE_DEFAULT_MIME_PROP] );
                item->write( 0, i.value().toUtf8() );
                items.append( item );
            }
            else
            {
                LOG_WARNING("Contact with id " << i.key() <<" doesn't exist!");
            }
        }
    }

    return items;
}

Buteo::StorageItem* ContactStorage::getItem( const QString& aItemId )
{
    FUNCTION_CALL_TRACE;

    if( !iBackend )
    {
        return NULL;
    }

    SimpleItem* newItem = NULL;

    QContactLocalId id;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    id = QContactId::fromString (aItemId);
#else
    id = aItemId.toUInt();
#endif
    QContact contact;

    iBackend->getContact( id, contact );
    QDateTime creationTime = iBackend->getCreationTime( contact );

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    if( iFreshItems.contains( id.toString () ) )
    {
        LOG_DEBUG( "Intercepted fresh item:" << id.toString () );
        iSnapshot.insert( id.toString (), creationTime );
        iFreshItems.removeOne( id.toString () );
    }
#else
    if( iFreshItems.contains( id ) )
    {
        LOG_DEBUG( "Intercepted fresh item:" << id );
        iSnapshot.insert( id, creationTime );
        iFreshItems.removeOne( id );
    }
#endif
    QString contactData = iBackend->convertQContactToVCard( contact );

    if(!contactData.isEmpty())
    {
        newItem = new SimpleItem;
        newItem->setId(aItemId);
        newItem->setType(iProperties[STORAGE_DEFAULT_MIME_PROP]);
        newItem->write(0,contactData.toUtf8());
    }
    else
    {
        LOG_WARNING( "Contact does not exist:" << aItemId );
    }

    return newItem;
}

ContactStorage::OperationStatus ContactStorage::addItem(Buteo::StorageItem& aItem)
{
    FUNCTION_CALL_TRACE;

    QList<Buteo::StorageItem*> items;
    items.append( &aItem );

    QList<ContactStorage::OperationStatus> status = addItems( items );

    return status.first();
}

QList<ContactStorage::OperationStatus> ContactStorage::addItems( const QList<Buteo::StorageItem*>& aItems )
{

    FUNCTION_CALL_TRACE;

    QList<ContactStorage::OperationStatus> storageErrorList;
    QDateTime currentTime = QDateTime::currentDateTime();

    if( !iBackend )
    {
        for ( int i = 0; i < aItems.size(); i++)
        {
            storageErrorList.append(STATUS_ERROR);
        }
        return storageErrorList;
    }

    QList<QString> contactsList;
    foreach(Buteo::StorageItem *item , aItems) {
        QByteArray data;
        item->read(0,item->getSize(),data);
        contactsList.append(QString::fromUtf8(data.data()));
    }

    QList<QString> contactsIdList;
    QMap<int, ContactsStatus> contactsErrorMap;
    bool retVal = iBackend->addContacts(contactsList, contactsErrorMap);

    if((contactsErrorMap.size()  != contactsList.size()) ||
        (contactsList.size() != aItems.size()))
    {

        LOG_WARNING("Something Wrong with Batch Addition in Contacts Backend : " << retVal);
        LOG_DEBUG("contactsErrorMap.size() " << contactsErrorMap.size() );
        LOG_DEBUG("contactsList.size()" << contactsList.size());
        LOG_DEBUG("aItems.size()" << aItems.size());

        for ( int i = 0; i < aItems.size(); i++) {
            storageErrorList.append(STATUS_ERROR);
        }

    }
    else
    {

        QMapIterator<int ,ContactsStatus> i(contactsErrorMap);
        int j = 0;
        while (i.hasNext()) {
            i.next();
            Buteo::StorageItem *item = aItems[j];
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            item->setId(i.value().id);
#else
            item->setId(QString::number(i.value().id));
#endif

            ContactStorage::OperationStatus status = mapErrorStatus(i.value().errorCode);

            if( status == STATUS_OK )
            {
                // This item was successfully added, so let's add it to the snapshot
                iSnapshot.insert( i.value().id, currentTime );
            }

            storageErrorList.append(status);
            j++;
        }

    }

    return storageErrorList;

}


ContactStorage::OperationStatus ContactStorage::modifyItem(Buteo::StorageItem& aItem)
{
        FUNCTION_CALL_TRACE;

        ContactStorage::OperationStatus status = STATUS_ERROR;

        if(iBackend ) {
                QString strID = aItem.getId();
                QByteArray data;
                aItem.read( 0, aItem.getSize(), data );
                QString Contact = QString::fromUtf8( data );
                qDebug()  << "Modifying an Item with data : " << Contact;
                qDebug() << "Modifying an Item with ID : "  << strID;
                QContactManager::Error error = iBackend->modifyContact(strID ,Contact);
                status = mapErrorStatus(error);
                qDebug()  << "After Modification String ID  is " << strID;
        }

        return status;
}

QList<ContactStorage::OperationStatus> ContactStorage::modifyItems(const QList<Buteo::StorageItem *> &aItems)
{
        FUNCTION_CALL_TRACE;
        QList<ContactStorage::OperationStatus> storageErrorList;

        qDebug()  << "Items to Modify :"  << aItems.size();

        if(iBackend) {

        QStringList contactsList;
        QStringList contactsIdList;
                foreach(Buteo::StorageItem *item , aItems) {
                        QByteArray data;
                        item->read(0,item->getSize(),data);
                        contactsList.append(QString::fromUtf8(data.data()));
                        contactsIdList.append(item->getId());
                }

        QMap<int, ContactsStatus> contactsErrorMap =
                iBackend->modifyContacts(contactsList, contactsIdList);

                if(contactsErrorMap.size()  != contactsList.size())
                {

                        LOG_WARNING("Something Wrong with Batch Mofication in Contacts Backend");
                        LOG_DEBUG("contactsErrroMap.size() " << contactsErrorMap.size() );
                        LOG_DEBUG("contactsList.size()" << contactsList.size() );
                        for ( int i = 0; i < aItems.size(); i++) {
                            storageErrorList.append(STATUS_ERROR);
                        }

                } else  {

            QMapIterator<int, ContactsStatus> i(contactsErrorMap);
                        int j = 0;
                        while (i.hasNext()) {
                                i.next();
                                Buteo::StorageItem *item = aItems[j];
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                                item->setId(i.value().id);
#else
                                item->setId(QString::number(i.value().id));
#endif
                                LOG_DEBUG("Id set in Storage " << item->getId());
                                storageErrorList.append(mapErrorStatus(i.value().errorCode));
                                j++;
                        }

                } // end if  contactsErrroMap.size()  != contactsList.size()

        } else {

                for ( int i = 0; i < aItems.size(); i++) {
                        storageErrorList.append(STATUS_ERROR);
                }

        }
        return storageErrorList;
}

ContactStorage::OperationStatus ContactStorage::deleteItem( const QString& aItemId )
{
    FUNCTION_CALL_TRACE;

    QList<QString> itemIds;
    itemIds.append( aItemId );

    QList<ContactStorage::OperationStatus> status = deleteItems( itemIds );

    return status.first();
}

QList<ContactStorage::OperationStatus> ContactStorage::deleteItems(const QList<QString>& aItemIds )
{
    FUNCTION_CALL_TRACE;

    QList<ContactStorage::OperationStatus> statusList;
    QDateTime currentTime = QDateTime::currentDateTime();

    if( !iBackend )
    {
        for ( int i = 0; i < aItemIds.size(); i++)
        {
            statusList.append(STATUS_ERROR);
        }
        return statusList;
    }

    QMap<int, ContactsStatus> errorMap = iBackend->deleteContacts(aItemIds);
    if(errorMap.size() !=aItemIds.size() )
    {
        LOG_WARNING("Something wrong with batch deletion of Contacts");
        LOG_DEBUG("contactsErrroMap.size() " << errorMap.size() );
        LOG_DEBUG("contactsList.size()" << aItemIds.size() );
        for ( int i = 0; i < aItemIds.size(); i++)
        {
            statusList.append(STATUS_ERROR);
        }
    }
    else
    {
        QList<QString> itemIds;
        QList<QDateTime> creationTimes;
        QList<QDateTime> deletionTimes;

        QMapIterator<int, ContactsStatus> i(errorMap);
        int j = 0;
        while( i.hasNext() )
        {
            i.next();
            ContactStorage::OperationStatus status = mapErrorStatus( i.value().errorCode );

            if( status == STATUS_OK )
            {

                // This item was successfully deleted, so let's remove it from the snapshot and
                // add it to the deleted items.

                QString itemId = aItemIds[j];

                itemIds.append( itemId );
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                creationTimes.append( iSnapshot.value( itemId ));
                iSnapshot.remove( itemId );
#else
                creationTimes.append( iSnapshot.value( itemId.toUInt()) );
                iSnapshot.remove( itemId.toUInt() );
#endif
                deletionTimes.append( currentTime );


            }

            statusList.append( status );
            j++;
        }

        if( !itemIds.isEmpty() )
        {
            iDeletedItems.addDeletedItems( itemIds, creationTimes, deletionTimes );
        }
    }


    return statusList;
}

bool ContactStorage::doInitItemAnalysis()
{
    /* Items are analyzed using two sets of items: those that existed at the end
     * of the previous time this plugin was uninitiated (called snapshot), and those
     * that exists now (called backend). The point of the analysis is to divide the
     * items into 3 groups:
     * 1) Items that exist only in the snapshot. These items have been deleted, and
     *    should be moved to the list of deleted items. The creation time of these
     *    items is known, as they are stored in the database.
     * 2) Items that exist both in the snapshot and the backend. Since there's no
     *    change in the lifetime of these items, they do not need any special
     *    attention.
     * 3) Items that exist only in the backed. These items have been created recently,
     *    so their creation time needs to be fetched from the backend before they
     *    are added to snapshot we are storing to database during uninit(). We could
     *    retrieve creation times here, but because these items are usually accessed
     *    anyway during a session, we will just add them to a list of "fresh items"
     *    which will be checked whenever a contact is retrieved from backend.
     *    The creation times of all remaining "fresh" items which were not accessed
     *    during the session will be retrieved in uninit()
     *
     * After items have been analyzed, the in-memory snapshot will be identical to
     * backend. In-memory snapshot is updated whenever we add or delete items. In
     * the item analysis of uninit() we therefore have an up-to-date snapshot of the
     * contents of the backend. Changes that external application do the beckend
     * during the sync session will get discovered in the next session.
     */

    FUNCTION_CALL_TRACE;

    Q_ASSERT( iBackend );
    iSnapshot.clear();
    iFreshItems.clear();

    QDateTime currentTime = QDateTime::currentDateTime();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QMap<QString, QDateTime> snapshot;
    QList<QString> backend;
    QList<QString> freshItems;
#else
    QMap<QContactLocalId, QDateTime> snapshot;
    QList<QContactLocalId> backend;
    QList<QContactLocalId> freshItems;
#endif

    // ** Retrieve previous snapshot from db and convert to QHash
    QList<QString> snapshotItems;
    QList<QDateTime> snapshotCreationTimes;

    if( !iDeletedItems.getSnapshot(snapshotItems, snapshotCreationTimes) ) {
        return false;
    }

    for( int i = 0; i < snapshotItems.count(); ++i )
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        snapshot.insert( snapshotItems[i], snapshotCreationTimes[i] );
#else
        snapshot.insert( snapshotItems[i].toUInt(), snapshotCreationTimes[i] );
#endif
    }

    // ** Retrieve backend
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QList<QContactId> backendIds = iBackend->getAllContactIds();
    foreach (const QContactId id, backendIds) {
        backend << id.toString ();
    }
#else
    backend = iBackend->getAllContactIds();
#endif

    LOG_DEBUG( "Found" << snapshot.count() << "items from snapshot" );
    LOG_DEBUG( "Found" << backend.count() << "items from backend" );

    QList<QString> itemIds;
    QList<QDateTime> creationTimes;
    QList<QDateTime> deletionTimes;

    // ** Find items only in the snapshot and mark them as deleted
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QMutableMapIterator<QString, QDateTime> i( snapshot );
#else
    QMutableMapIterator<QContactLocalId, QDateTime> i( snapshot );
#endif

    while( i.hasNext() )
    {
        i.next();
        if( !backend.contains( i.key() ) )
        {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            itemIds.append( i.key() );
#else
            itemIds.append( QString::number( i.key() ) );
#endif
            creationTimes.append( i.value() );
            deletionTimes.append( currentTime );
            i.remove();
        }
    }

    LOG_DEBUG( "Detected" << itemIds.count() <<"deleted items" );

    if( !itemIds.isEmpty() )
    {
        iDeletedItems.addDeletedItems( itemIds, creationTimes, deletionTimes );
    }

    // ** Find items only in backend and mark them as fresh items

    for( int i = 0; i < backend.count(); ++i )
    {
        if( !snapshot.contains( backend[i] ) )
        {
            freshItems.append( backend[i] );
            snapshot.insert( backend[i], QDateTime() );
        }
    }

    iSnapshot = snapshot;
    iFreshItems = freshItems;

    LOG_DEBUG( "Detected" << iFreshItems.count() <<"fresh items" );

    return true;

}

bool ContactStorage::doUninitItemAnalysis()
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( iBackend );

    // ** Retrieve creation times for the remaining "fresh" items from the backend

    if( !iFreshItems.isEmpty() )
    {
        LOG_DEBUG( "Retrieving creation times for" << iFreshItems.count() << "fresh items" );

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QList<QContactId> idList;
        foreach (const QString freshItem, iFreshItems) {
            idList << QContactId::fromString (freshItem);
        }
        QList<QDateTime> freshCreationTimes = iBackend->getCreationTimes( idList );
#else
        QList<QDateTime> freshCreationTimes = iBackend->getCreationTimes( iFreshItems );
#endif

        for( int i = 0; i < iFreshItems.count(); ++i )
        {
            iSnapshot.insert( iFreshItems[i], freshCreationTimes[i] );
        }
    }

    // ** Store the snapshot to the database

    QTime timer;
    timer.start();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QList<QString> intIds = iSnapshot.keys();
#else
    QList<QContactLocalId> intIds = iSnapshot.keys();
#endif

    QList<QString> ids;
    for( int i = 0; i < intIds.count(); ++i )
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        ids.append( intIds[i] );
#else
        ids.append( QString::number( intIds[i] ) );
#endif
    }

    QList<QDateTime> creationTimes = iSnapshot.values();

    LOG_DEBUG( "Storing" << ids.count() <<"items to snapshot" );

    iDeletedItems.setSnapshot( ids, creationTimes );

    iSnapshot.clear();
    iFreshItems.clear();

    return true;
}


QList<Buteo::StorageItem*> ContactStorage::getStoreList(QList<QContactLocalId> &aStrIDList)
{
        FUNCTION_CALL_TRACE;

        QList<Buteo::StorageItem*> itemList;


    if (iBackend != NULL) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QMap<QString,QString> idDataMap;
#else
        QMap<QContactLocalId,QString> idDataMap;
#endif
        iBackend->getContacts(aStrIDList, idDataMap);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QMapIterator<QString, QString > iter(idDataMap);
#else
        QMapIterator<QContactLocalId, QString > iter(idDataMap);
#endif

        while (iter.hasNext()) {
            iter.next();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            SimpleItem* item = convertVcardToStorageItem(QContactId::fromString (iter.key()), iter.value());
#else
            SimpleItem* item = convertVcardToStorageItem(iter.key(), iter.value());
#endif
            if (item  != NULL) {
                itemList.append(item);
            }

        }
    }

    return itemList;
}

QByteArray ContactStorage::getCtCaps( const QString& aFilename ) const
{
    FUNCTION_CALL_TRACE;

    QFile ctCapsFile( SyncMLConfig::getXmlDataPath() + aFilename  );
    QByteArray ctCaps;

    if( ctCapsFile.open(QIODevice::ReadOnly)) {
        ctCaps = ctCapsFile.readAll();
        ctCapsFile.close();
    } else {
        qWarning()  << "Failed to open CTCaps file for contacts storage:" << aFilename ;
    }

    return ctCaps;
}

ContactStorage::OperationStatus ContactStorage::mapErrorStatus\
(const QContactManager::Error &aContactError) const
{
        ContactStorage::OperationStatus iStorageStatus = STATUS_ERROR;
        switch(aContactError) {
        case QContactManager::NoError:
                iStorageStatus = STATUS_OK;
                break;

        case QContactManager::DoesNotExistError:
                iStorageStatus = STATUS_NOT_FOUND;
                break;

        case QContactManager::AlreadyExistsError:
                iStorageStatus = STATUS_DUPLICATE;
                break;

        case QContactManager::InvalidDetailError:
        case QContactManager::VersionMismatchError:
        case QContactManager::InvalidContactTypeError:
                iStorageStatus = STATUS_INVALID_FORMAT;
                break;

        case QContactManager::OutOfMemoryError:
                iStorageStatus = STATUS_STORAGE_FULL;
                break;

        case QContactManager::LimitReachedError:
                iStorageStatus = STATUS_OBJECT_TOO_BIG;
                break;

        case QContactManager::InvalidRelationshipError:
        case QContactManager::LockedError:
        case QContactManager::DetailAccessError:
        case QContactManager::PermissionsError:
        case QContactManager::NotSupportedError:
        case QContactManager::BadArgumentError:
        case QContactManager::UnspecifiedError:
        default:
                iStorageStatus = STATUS_ERROR;
                break;
        }
        return iStorageStatus;
}

/*!
    \fn ContactStorage::convertVcardToStorageItem(const QContactLocalId, const QString&)
 */
SimpleItem* ContactStorage::convertVcardToStorageItem(const QContactLocalId aItemKey,
                                                      const QString& aItemData)
{

    FUNCTION_CALL_TRACE;

    SimpleItem* storageItem = new SimpleItem;

    if(storageItem != NULL) {
        qDebug() << "ID is " << aItemKey;
        qDebug() << "Data is " << aItemData;
        storageItem->write( 0, aItemData.toUtf8() );
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        storageItem->setId(aItemKey.toString());
#else
        storageItem->setId(QString::number(aItemKey));
#endif
        storageItem->setType(iProperties[STORAGE_DEFAULT_MIME_PROP]);
    }
    else {
        qWarning() << "Memory Allocation Failed";
    }

    return storageItem;
}
