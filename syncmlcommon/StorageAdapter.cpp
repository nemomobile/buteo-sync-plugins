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

#include "StorageAdapter.h"

#include <libsyncpluginmgr/StoragePlugin.h>
#include <libsyncpluginmgr/StorageItem.h>

#include "SyncMLCommon.h"
#include "ItemAdapter.h"
#include "SyncMLConfig.h"

#include <LogMacros.h>


// Database file for SyncML storage adapter database
#define  ADAPTERDBFILE  "syncmladapter.db"

StorageAdapter::StorageAdapter( Buteo::StoragePlugin* aPlugin )
 : iPlugin( aPlugin )
{
    FUNCTION_CALL_TRACE;

}

StorageAdapter::~StorageAdapter()
{
    FUNCTION_CALL_TRACE;
}

bool StorageAdapter::isValid()
{
    FUNCTION_CALL_TRACE;

    if( iPlugin ) {
        return true;
    }
    else {
        return false;
    }
}

bool StorageAdapter::init()
{
    FUNCTION_CALL_TRACE;

    // ** Process properties from client/server plugin

    // ** Process properties from storage plugin
    QMap<QString, QString> pluginProperties;

    iPlugin->getProperties( pluginProperties );
    LOG_DEBUG( "StorageAdapter: dumping storage properties:" );
    LOG_DEBUG( pluginProperties );

    // Preferred format

    QString preferredFormat = pluginProperties.value( STORAGE_DEFAULT_MIME_PROP );
    QString preferredVersion = pluginProperties.value( STORAGE_DEFAULT_MIME_VERSION_PROP );
    if( preferredFormat.isEmpty() || preferredVersion.isEmpty() ) {
        LOG_CRITICAL( "No STORAGE_DEFAULT_MIME_PROP or STORAGE_DEFAULT_MIME_VERSION_PROP"
                      <<"found for storage: " << iPlugin->getPluginName() );
        return false;
    }

    // Currently we support only one format per storage
    DataSync::ContentFormat format;
    format.iType = preferredFormat;
    format.iVersion = preferredVersion;
    iFormats.setPreferredRx( format );
    iFormats.setPreferredTx( format );
    iFormats.rx().append( format );
    iFormats.tx().append( format );

    // Source URI

    iSourceDB = pluginProperties.value( STORAGE_SOURCE_URI );

    if( iSourceDB.isEmpty() ) {
        LOG_CRITICAL( "No STORAGE_SOURCE_URI prop found for storage: " << iPlugin->getPluginName() );
        return false;
    }

    // Target URI

    iTargetDB = pluginProperties[STORAGE_REMOTE_URI];

    // ** Own initialization

    iType = preferredFormat;

    QString dbFilePath = SyncMLConfig::getDatabasePath() + ADAPTERDBFILE;
    iIdMapper.init( dbFilePath, iPlugin->getPluginName() );

    return true;

}

bool StorageAdapter::uninit()
{
    FUNCTION_CALL_TRACE;

    iIdMapper.uninit();

    return true;
}

Buteo::StoragePlugin* StorageAdapter::getPlugin() const
{
    FUNCTION_CALL_TRACE;

    return iPlugin;
}

const QString& StorageAdapter::getSourceURI() const
{
    FUNCTION_CALL_TRACE;

    return iSourceDB;
}

const DataSync::StorageContentFormatInfo& StorageAdapter::getFormatInfo() const
{
    FUNCTION_CALL_TRACE;

    return iFormats;
}

qint64 StorageAdapter::getMaxObjSize() const
{
    FUNCTION_CALL_TRACE;

    // Max object size not supported at the moment
    return 0;
}

QByteArray StorageAdapter::getPluginCTCaps( DataSync::ProtocolVersion aVersion ) const
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( iPlugin );

    if( aVersion == DataSync::SYNCML_1_1 ) {
        return iPlugin->getProperty( STORAGE_SYNCML_CTCAPS_PROP_11 ).toAscii();
    }
    else if( aVersion == DataSync::SYNCML_1_2 ) {
        return iPlugin->getProperty( STORAGE_SYNCML_CTCAPS_PROP_12 ).toAscii();
    }
    else {
        Q_ASSERT( 0 );
        return "";
    }
}

bool StorageAdapter::getAll( QList<DataSync::SyncItemKey>& aKeys )
{
    FUNCTION_CALL_TRACE;

    QList<QString> newKeys;
    if (!iPlugin->getAllItemIds( newKeys )) {
        return false;
    }

    foreach (const QString &key, newKeys) {
        aKeys.append(iIdMapper.value(key));
    }

    return true;
}

bool StorageAdapter::getModifications( QList<DataSync::SyncItemKey>& aNewKeys,
                                       QList<DataSync::SyncItemKey>& aReplacedKeys,
                                       QList<DataSync::SyncItemKey>& aDeletedKeys,
                                       const QDateTime& aTimeStamp )
{
    FUNCTION_CALL_TRACE;

    QList<QString> newKeys;
    QList<QString> replacedKeys;
    QList<QString> deletedKeys;

    if( !iPlugin->getNewItemIds( newKeys, aTimeStamp ) ||
        !iPlugin->getModifiedItemIds( replacedKeys, aTimeStamp ) ||
        !iPlugin->getDeletedItemIds( deletedKeys, aTimeStamp ) ) {
        return false;
    }

    for( int i = 0; i < newKeys.count(); ++i ) {
        aNewKeys.append( iIdMapper.value( newKeys[i] ) );
    }

    for( int i = 0; i < replacedKeys.count(); ++i ) {
        aReplacedKeys.append( iIdMapper.value( replacedKeys[i] ) );
    }

    for( int i = 0; i < deletedKeys.count(); ++i ) {
        aDeletedKeys.append( iIdMapper.value( deletedKeys[i] ) );
    }

    return true;

}

DataSync::SyncItem* StorageAdapter::newItem()
{
    FUNCTION_CALL_TRACE;

    Buteo::StorageItem* item = iPlugin->newItem();

    if( item ) {

        ItemAdapter* adapter = new ItemAdapter( item );
        adapter->setKey( "" );
        adapter->setType( iType );
        return adapter;
    }
    else {
        return NULL;
    }
}

DataSync::SyncItem* StorageAdapter::getSyncItem( const DataSync::SyncItemKey& aKey )
{
    FUNCTION_CALL_TRACE;

    QString id = iIdMapper.key( aKey );

    Buteo::StorageItem* item = iPlugin->getItem( id );

    if( item ) {
        ItemAdapter* adapter = new ItemAdapter( item );
        adapter->setKey( aKey );
        adapter->setType( item->getType() );

        if( !item->getParentId().isEmpty() ) {
            adapter->setParentKey( iIdMapper.value( item->getParentId() ) );
        }

        return adapter;
    }
    else {
        return NULL;
    }

}

QList<DataSync::SyncItem*> StorageAdapter::getSyncItems( const QList<DataSync::SyncItemKey>& aKeyList )
{
    FUNCTION_CALL_TRACE;

    QStringList idList;
    QList<DataSync::SyncItemKey>::const_iterator i;
        for( i = aKeyList.constBegin(); i != aKeyList.constEnd(); ++i)
    {
        idList.append( iIdMapper.key( *i ) );
    }

    QList<Buteo::StorageItem*> items = iPlugin->getItems( idList );
    QList<DataSync::SyncItem*> adapters;

    QList<Buteo::StorageItem*>::const_iterator j;
    for( j = items.constBegin(); j != items.constEnd(); ++j)
    {
        if( *j )
        {
            ItemAdapter* adapter = new ItemAdapter( *j );
            adapter->setKey( iIdMapper.value( (*j)->getId() ) );
            adapter->setType( (*j)->getType() );
            if( !(*j)->getParentId().isEmpty() )
            {
                adapter->setParentKey( iIdMapper.value( (*j)->getParentId() ) );
            }
            adapters.append( adapter );
        }
        else
        {
            adapters.append( NULL );
        }
    }

    return adapters;
}

QList<DataSync::StoragePlugin::StoragePluginStatus> StorageAdapter::addItems( const QList<DataSync::SyncItem*>& aItems )
{

    FUNCTION_CALL_TRACE;

    QList<StoragePlugin::StoragePluginStatus> results;
    QList<Buteo::StorageItem*> items;

    for( int i = 0; i < aItems.count(); ++i ) {
        items.append( toStorageItem( aItems[i] ) );
    }

    QList< Buteo::StoragePlugin::OperationStatus > operations;
    if( aItems.count() )
    {
        operations = iPlugin->addItems( items );
    }

    for( int i = 0; i < operations.count(); ++i ) {
        StoragePlugin::StoragePluginStatus status = convertStatus( operations[i] );

        if( status == STATUS_OK ) {
            QString mappedId = iIdMapper.value( items[i]->getId() );
            ItemAdapter* adapter = static_cast<ItemAdapter*>( aItems[i] );
            adapter->setKey( mappedId );
        }

        results.append( status );

    }

    return results;

}

QList<DataSync::StoragePlugin::StoragePluginStatus> StorageAdapter::replaceItems( const QList<DataSync::SyncItem*>& aItems )
{

    FUNCTION_CALL_TRACE;

    QList<StoragePlugin::StoragePluginStatus> results;
    QList<Buteo::StorageItem*> items;

    // There is no need to do item id mapping here, as the StorageItems always have their correct id's.
    // Only ItemAdapter houses mapped id's.
    for( int i = 0; i < aItems.count(); ++i ) {
        items.append( toStorageItem( aItems[i] ) );
    }

    QList< Buteo::StoragePlugin::OperationStatus > operations;
    if( aItems.count() )
    {
        operations = iPlugin->modifyItems( items );
    }

    for( int i = 0; i < operations.count(); ++i ) {
        StoragePlugin::StoragePluginStatus status = convertStatus( operations[i] );

        if( status == STATUS_OK ) {
            QString mappedId = iIdMapper.value( items[i]->getId() );
            ItemAdapter* adapter = static_cast<ItemAdapter*>( aItems[i] );
            adapter->setKey( mappedId );
        }

        results.append( status );

    }

    return results;

}

QList<DataSync::StoragePlugin::StoragePluginStatus> StorageAdapter::deleteItems( const QList<DataSync::SyncItemKey>& aKeys )
{
    FUNCTION_CALL_TRACE;

    QList<QString> ids;
    QList<StoragePlugin::StoragePluginStatus> results;

    // aKeys houses mapped id's, so they must be converted back to actual item id's
    for( int i = 0; i < aKeys.count(); ++i ) {
        ids.append( iIdMapper.key( aKeys[i] ) );
    }

    QList< Buteo::StoragePlugin::OperationStatus > operations;
    if( aKeys.count() )
    {
        operations = iPlugin->deleteItems( ids );
    }

    for( int i = 0; i < operations.count(); ++i ) {
        StoragePlugin::StoragePluginStatus status = convertStatus( operations[i] );

        results.append( status );

    }

    return results;
}

DataSync::StoragePlugin::StoragePluginStatus StorageAdapter::convertStatus( Buteo::StoragePlugin::OperationStatus aStatus ) const
{

    StoragePlugin::StoragePluginStatus status;

    switch( aStatus )
    {
        case Buteo::StoragePlugin::STATUS_OK:
        {
            status = STATUS_OK;
            break;
        }
        case Buteo::StoragePlugin::STATUS_NOT_FOUND:
        {
            status = STATUS_NOT_FOUND;
            break;
        }
        case Buteo::StoragePlugin::STATUS_DUPLICATE:
        {
            status = STATUS_DUPLICATE;
            break;
        }
        case Buteo::StoragePlugin::STATUS_OBJECT_TOO_BIG:
        {
            status = STATUS_OBJECT_TOO_BIG;
            break;
        }
        case Buteo::StoragePlugin::STATUS_STORAGE_FULL:
        {
            status = STATUS_STORAGE_FULL;
            break;
        }
        case Buteo::StoragePlugin::STATUS_INVALID_FORMAT:
        {
            status = STATUS_INVALID_FORMAT;
            break;
        }
        case Buteo::StoragePlugin::STATUS_ERROR:
        default:
        {
            status = STATUS_ERROR;
            break;
        }

    }

    return status;

}

Buteo::StorageItem* StorageAdapter::toStorageItem( const DataSync::SyncItem* aSyncItem ) const
{
    FUNCTION_CALL_TRACE;

    const ItemAdapter* adapter = static_cast<const ItemAdapter*>( aSyncItem );
    Buteo::StorageItem& item = adapter->getItem();

    item.setType( adapter->getType() );
    item.setParentId( *adapter->getParentKey() );
    item.setVersion( adapter->getVersion() );

    return &item;
}
