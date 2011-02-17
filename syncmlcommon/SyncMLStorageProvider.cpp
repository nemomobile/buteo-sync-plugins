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

#include "SyncMLStorageProvider.h"

#include <libsyncprofile/Profile.h>
#include <libsyncprofile/ProfileEngineDefs.h>

#include <libsyncpluginmgr/PluginCbInterface.h>
#include <libsyncpluginmgr/StoragePlugin.h>

#include <libmeegosyncml/StoragePlugin.h>
#include <libmeegosyncml/SessionHandler.h>

#include "SyncMLCommon.h"
#include "StorageAdapter.h"

#include <LogMacros.h>

SyncMLStorageProvider::SyncMLStorageProvider()
 : iProfile( 0 ), iPlugin( 0 ), iCbInterface( 0 ), iRequestStorages( false )
{
    FUNCTION_CALL_TRACE;
}

SyncMLStorageProvider::~SyncMLStorageProvider()
{
    FUNCTION_CALL_TRACE;
}

bool SyncMLStorageProvider::init( Buteo::Profile* aProfile,
                                  Buteo::SyncPluginBase* aPlugin,
                                  Buteo::PluginCbInterface* aCbInterface,
                                  bool aRequestStorages )
{
    FUNCTION_CALL_TRACE;

    if( !aProfile || !aPlugin || !aCbInterface ) {
        LOG_CRITICAL( "NULL parameters passed to init()" );
        return false;
    }

    iProfile = aProfile;
    iPlugin = aPlugin;
    iCbInterface = aCbInterface;
    iRequestStorages = aRequestStorages;

    return true;
}

bool SyncMLStorageProvider::uninit()
{
    FUNCTION_CALL_TRACE;

    return true;
}

QString SyncMLStorageProvider::getPreferredURINames( const QString &aURI )
{
    Q_UNUSED(aURI);	
   // TODO : Handle possible multiple URI names in storage profiles
   // One way to do this is to have separators for Local URI, like ./notes;./Notepad
   return QString();
}

bool SyncMLStorageProvider::getStorageContentFormatInfo( const QString& aURI,
                                                         DataSync::StorageContentFormatInfo& aInfo )
{
    FUNCTION_CALL_TRACE;

    const Buteo::Profile* storageProfile =
            iProfile->subProfileByKeyValue( STORAGE_SOURCE_URI, aURI,
                                            Buteo::Profile::TYPE_STORAGE, true );

    if( !storageProfile ) {
        LOG_DEBUG( "Could not find storage for URI" << aURI );
        return false;
    }

    QString preferredFormat = storageProfile->key( STORAGE_DEFAULT_MIME_PROP );
    QString preferredVersion = storageProfile->key( STORAGE_DEFAULT_MIME_VERSION_PROP );

    // Currently we support only one format per storage
    DataSync::ContentFormat format;
    format.iType = preferredFormat;
    format.iVersion = preferredVersion;
    aInfo.setPreferredRx( format );
    aInfo.setPreferredTx( format );
    aInfo.rx().append( format );
    aInfo.tx().append( format );

    return true;
}

DataSync::StoragePlugin* SyncMLStorageProvider::acquireStorageByURI( const QString& aURI )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Incoming request to acquire storage by URI:" << aURI );

#if 0
    QString preferredURI = getPreferredURINames( aURI );
#endif

    const Buteo::Profile* storageProfile =
            iProfile->subProfileByKeyValue( STORAGE_SOURCE_URI, aURI,
                                            Buteo::Profile::TYPE_STORAGE, true );

    if( !storageProfile ) {
        LOG_DEBUG( "Could not find storage for URI" << aURI );
        return NULL;
    }

    LOG_DEBUG( "Found storage for URI" << aURI << ":" << storageProfile->name() );

    return acquireStorage( storageProfile );
}

DataSync::StoragePlugin* SyncMLStorageProvider::acquireStorageByMIME( const QString& aMIME )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Incoming request to acquire storage by MIME:" << aMIME );

    const Buteo::Profile* storageProfile =
            iProfile->subProfileByKeyValue( STORAGE_DEFAULT_MIME_PROP, aMIME,
                                            Buteo::Profile::TYPE_STORAGE, true );

    if( !storageProfile ) {
        LOG_DEBUG( "Could not find storage for MIME" << aMIME );
        return NULL;
    }

    LOG_DEBUG( "Found storage for MIME" << aMIME << ":" << storageProfile->name() );

    return acquireStorage( storageProfile );

}

void SyncMLStorageProvider::releaseStorage( DataSync::StoragePlugin* aStorage )
{
    FUNCTION_CALL_TRACE;

    if (!aStorage)
        return;

    LOG_DEBUG( "Incoming request to release storage with source URI" << aStorage->getSourceURI() );

    StorageAdapter* adapter = static_cast<StorageAdapter*>( aStorage );

    if( !adapter->uninit() ) {
        LOG_WARNING( "Storage adapter uninitialization failed" );
    }

    Buteo::StoragePlugin* storage = adapter->getPlugin();

    QString backend = storage->getProperty( Buteo::KEY_BACKEND );

    if( !storage->uninit() ) {
        LOG_WARNING( "Storage uninitialization failed" );
    }

    iCbInterface->destroyStorage( storage );

    if( iRequestStorages ) {
        iCbInterface->releaseStorage( backend, iPlugin );
    }

    delete aStorage;

}

DataSync::StoragePlugin* SyncMLStorageProvider::acquireStorage( const Buteo::Profile* aProfile )
{
    FUNCTION_CALL_TRACE;

    if (!aProfile)
        return NULL;

    QString backend = aProfile->key( Buteo::KEY_BACKEND, aProfile->name() );
    QString pluginName = aProfile->key( Buteo::KEY_PLUGIN, aProfile->name() );
    QString uuid = iProfile->key(Buteo::KEY_UUID);
    QString remoteName = iProfile->key(Buteo::KEY_REMOTE_NAME);

    if( iRequestStorages && !iCbInterface->requestStorage( backend, iPlugin ) ) {
        LOG_CRITICAL( "Could not reserve storage backend:" << backend );
    }

    Buteo::StoragePlugin* storage = iCbInterface->createStorage( pluginName );

    if( !storage ) {
        iCbInterface->releaseStorage( backend, iPlugin );
        LOG_DEBUG( "Could not create storage:" << pluginName );
        return NULL;
    }

    // Make sure that the backend name that was used in storage reservation is
    // saved to the properties of storage plug-in. This name must be used again
    // when the storage backend is released.
    QMap<QString, QString> keys = aProfile->allKeys();
    keys.insert( Buteo::KEY_BACKEND, backend );
    keys.insert(Buteo::KEY_UUID, uuid);
    keys.insert(Buteo::KEY_REMOTE_NAME, remoteName);

    // If protocol version is not defined in the keys read from profile, try to
    // read the version from the session handler and insert a corresponding key,
    // so that storage plug-in knows which protocol version is in use.
    if (!keys.contains(PROF_SYNC_PROTOCOL) && iSessionHandler != 0)
    {
        if (iSessionHandler->getProtocolVersion() == DataSync::SYNCML_1_1)
        {
            keys[PROF_SYNC_PROTOCOL] = SYNCML11;
        }
        else
        {
            keys[PROF_SYNC_PROTOCOL] = SYNCML12;
        }
    }

    if( !storage->init( keys ) ) {
        LOG_DEBUG( "Could not initialize storage:" << pluginName );
        iCbInterface->destroyStorage( storage );
        iCbInterface->releaseStorage( backend, iPlugin );
        return NULL;
    }

    StorageAdapter* adapter = new StorageAdapter( storage );

    if( !adapter->init() ) {

        LOG_DEBUG("Initialization of adapter for storage" << pluginName << "FAILED" );
        iCbInterface->destroyStorage( storage );
        iCbInterface->releaseStorage( backend, iPlugin );
        delete adapter;
        return NULL;
    }

    return adapter;

}
