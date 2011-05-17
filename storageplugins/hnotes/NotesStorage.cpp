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

#include "NotesStorage.h"

#include <QFile>
#include <QStringListIterator>

#include <libsyncpluginmgr/StorageItem.h>

#include <LogMacros.h>

#include "SyncMLCommon.h"
#include "SyncMLConfig.h"

// @todo: Because CalendarMaemo does not support batched operations ( or it does
//        but we can't use it as we cannot retrieve the id's of committed items ),
//        batched operations are currently done in series.

const char* CTCAPSFILENAME11        = "CTCaps_notes_11.xml";
const char* CTCAPSFILENAME12        = "CTCaps_notes_12.xml";

const char* STORAGE_NOTEBOOK_PROP   = "Notebook Name";

const char* DEFAULT_TYPE            = "text/plain";
const char* DEFAULT_TYPE_VERSION    = "1.0";
const char* DEFAULT_NOTEBOOK        = "Personal";
const char* DEFAULT_NOTEBOOK_NAME   = "myNotebook";

extern "C" Buteo::StoragePlugin* createPlugin( const QString& aPluginName )
{
    return new NotesStorage( aPluginName );
}

extern "C" void destroyPlugin( Buteo::StoragePlugin* aStorage )
{
    delete aStorage;
}


NotesStorage::NotesStorage( const QString& aPluginName ) : Buteo::StoragePlugin( aPluginName ), iCommitNow( true )
{
    FUNCTION_CALL_TRACE;
}

NotesStorage::~NotesStorage()
{
    FUNCTION_CALL_TRACE;
}


bool NotesStorage::init( const QMap<QString, QString>& aProperties )
{
    FUNCTION_CALL_TRACE;

    iProperties                                = aProperties;
    iProperties[STORAGE_SYNCML_CTCAPS_PROP_11] = getCTCaps( CTCAPSFILENAME11 );
    iProperties[STORAGE_SYNCML_CTCAPS_PROP_12] = getCTCaps( CTCAPSFILENAME12 );

    LOG_DEBUG("Initializing notes");

    if( iProperties.value( STORAGE_DEFAULT_MIME_PROP ).isEmpty() ) {
        LOG_WARNING( STORAGE_DEFAULT_MIME_PROP << "property not found"
                     << "for notes storage, using default of" << DEFAULT_TYPE );
        iProperties[STORAGE_DEFAULT_MIME_PROP] = DEFAULT_TYPE;
    }

    return iBackend.init(iProperties[STORAGE_DEFAULT_MIME_PROP]);
}

bool NotesStorage::uninit()
{
    FUNCTION_CALL_TRACE;

    return iBackend.uninit();
}

bool NotesStorage::getAllItems( QList<Buteo::StorageItem*>& aItems )
{
    return iBackend.getAllNotes( aItems );
}

bool NotesStorage::getAllItemIds( QList<QString>& aItemIds )
{
    return iBackend.getAllNoteIds( aItemIds );
}

bool NotesStorage::getNewItems( QList<Buteo::StorageItem*>& aNewItems, const QDateTime& aTime )
{
    return iBackend.getNewNotes( aNewItems, normalizeTime( aTime ) );
}

bool NotesStorage::getNewItemIds( QList<QString>& aNewItemIds, const QDateTime& aTime )
{
    return iBackend.getNewNoteIds( aNewItemIds, normalizeTime( aTime ) );
}

bool NotesStorage::getModifiedItems( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime )
{
    return iBackend.getModifiedNotes( aModifiedItems, normalizeTime( aTime ) );
}

bool NotesStorage::getModifiedItemIds( QList<QString>& aModifiedItemIds, const QDateTime& aTime )
{
    return iBackend.getModifiedNoteIds( aModifiedItemIds, normalizeTime( aTime ) );
}

bool NotesStorage::getDeletedItemIds( QList<QString>& aDeletedItemIds, const QDateTime& aTime )
{
    return iBackend.getDeletedNoteIds( aDeletedItemIds, normalizeTime( aTime ) );
}

Buteo::StorageItem* NotesStorage::newItem()
{
    return iBackend.newItem();
}

QList<Buteo::StorageItem*> NotesStorage::getItems( const QStringList& aItemIdList )
{
    FUNCTION_CALL_TRACE;

    QList<Buteo::StorageItem*> items;
    QStringListIterator itr( aItemIdList );

    while( itr.hasNext() )
    {
        items.append( iBackend.getItem( itr.next() ) );
    }

    return items;
}

Buteo::StorageItem* NotesStorage::getItem( const QString& aItemId )
{
    return iBackend.getItem( aItemId );
}

Buteo::StoragePlugin::OperationStatus NotesStorage::addItem( Buteo::StorageItem& aItem )
{
    FUNCTION_CALL_TRACE;

    if( iBackend.addNote( aItem, iCommitNow ) ) {
        return STATUS_OK;
    }
    else {
        return STATUS_ERROR;
    }

}

QList<Buteo::StoragePlugin::OperationStatus> NotesStorage::addItems( const QList<Buteo::StorageItem*>& aItems )
{
    FUNCTION_CALL_TRACE;

    QList<OperationStatus> results;

    //Commit once at the end of the batch update
    iCommitNow = false;

    for( int i = 0; i < aItems.count(); ++i ) {
        results.append( addItem( *aItems[i] ) );
    }

    iCommitNow = true;
    bool saved = iBackend.commitChanges();
    Q_UNUSED( saved );

    return results;
}

Buteo::StoragePlugin::OperationStatus NotesStorage::modifyItem( Buteo::StorageItem& aItem )
{
    FUNCTION_CALL_TRACE;

    if( iBackend.modifyNote( aItem, iCommitNow ) ) {
        return STATUS_OK;
    }
    else {
        return STATUS_ERROR;
    }

}

QList<Buteo::StoragePlugin::OperationStatus> NotesStorage::modifyItems( const QList<Buteo::StorageItem*>& aItems )
{
    FUNCTION_CALL_TRACE;

    QList<OperationStatus> results;

    //Commit once at the end of the batch update
    iCommitNow = false;

    for( int i = 0; i < aItems.count(); ++i ) {
        results.append( modifyItem( *aItems[i] ) );
    }

    iCommitNow = true;
    bool saved = iBackend.commitChanges();
    Q_UNUSED( saved );

    return results;
}

Buteo::StoragePlugin::OperationStatus NotesStorage::deleteItem( const QString& aItemId )
{
    FUNCTION_CALL_TRACE;

    if( iBackend.deleteNote( aItemId, iCommitNow ) ) {
        return STATUS_OK;
    }
    else {
        return STATUS_ERROR;
    }

}

QList<Buteo::StoragePlugin::OperationStatus> NotesStorage::deleteItems( const QList<QString>& aItemIds )
{
    FUNCTION_CALL_TRACE;

    QList<OperationStatus> results;

    //Commit once at the end of the batch update
    iCommitNow = false;

    for( int i = 0; i < aItemIds.count(); ++i ) {
        results.append( deleteItem( aItemIds[i] ) );
    }

    iCommitNow = true;
    bool saved = iBackend.commitChanges();
    Q_UNUSED( saved );

    return results;
}

QDateTime NotesStorage::normalizeTime( const QDateTime& aTime ) const
{
    FUNCTION_CALL_TRACE;

    QDateTime normTime = aTime;

    QTime time = aTime.time();
    time.setHMS( time.hour(), time.minute(), time.second(), 0 );

    normTime.setTime( time );

    normTime = normTime.toUTC();

    return normTime;
}

QByteArray NotesStorage::getCTCaps( const QString& aFilename ) const
{
    FUNCTION_CALL_TRACE;

    QFile ctCapsFile( SyncMLConfig::getXmlDataPath() + aFilename  );
    QByteArray ctCaps;

    if( ctCapsFile.open(QIODevice::ReadOnly)) {
       ctCaps = ctCapsFile.readAll();
       ctCapsFile.close();
    } else {
        LOG_WARNING("Failed to open CTCaps file for notes storage:" << aFilename );
    }

    return ctCaps;

}
