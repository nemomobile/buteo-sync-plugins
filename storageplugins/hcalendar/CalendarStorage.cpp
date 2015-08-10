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

#include "CalendarStorage.h"

#include <QFile>
#include <QStringListIterator>

#include "SimpleItem.h"
#include "SyncMLCommon.h"
#include "SyncMLConfig.h"

#include <LogMacros.h>

// @todo: Because CalendarMaemo does not support batched operations ( or it does
//        but we can't use it as we cannot retrieve the id's of committed items ),
//        batched operations are currently done in series.

const char* CTCAPSFILENAME11 = "CTCaps_calendar_11.xml";
const char* CTCAPSFILENAME12 = "CTCaps_calendar_12.xml";

extern "C" Buteo::StoragePlugin* createPlugin( const QString& aPluginName )
{
    return new CalendarStorage( aPluginName );
}

extern "C" void destroyPlugin( Buteo::StoragePlugin *aStorage)
{
    delete aStorage;
}

CalendarStorage::CalendarStorage( const QString& aPluginName )
: Buteo::StoragePlugin(aPluginName)
{
    FUNCTION_CALL_TRACE;

    iCommitNow = true;
    iStorageType = VCALENDAR_FORMAT;
}

CalendarStorage::~CalendarStorage()
{
	FUNCTION_CALL_TRACE;
}

bool CalendarStorage::init( const QMap<QString, QString>& aProperties )
{
    FUNCTION_CALL_TRACE;

    iProperties = aProperties;

    // Note: we don't use the KEY_UUID value, as msyncd just generates
    // a random one on the fly - it doesn't actually correspond to any
    // real notebook UID which exists on the device.
    if( !iCalendar.init( ) ) {
        return false;
    }

    if( iProperties[CALENDAR_FORMAT] == CALENDAR_FORMAT_ICAL )
    {
        LOG_DEBUG("The calendar storage is using icalendar format");
        iStorageType = ICALENDAR_FORMAT;
        iProperties[STORAGE_DEFAULT_MIME_PROP] = "text/calendar";
        iProperties[STORAGE_DEFAULT_MIME_VERSION_PROP] = "2.0";
    }
    else
    {
        LOG_DEBUG("The calendar storage is using vcalendar format");
        iStorageType = VCALENDAR_FORMAT;
    }

    iProperties[STORAGE_SYNCML_CTCAPS_PROP_11] = getCtCaps( CTCAPSFILENAME11 );
    iProperties[STORAGE_SYNCML_CTCAPS_PROP_12] = getCtCaps( CTCAPSFILENAME12 );

    return true;
}

bool CalendarStorage::uninit()
{
    FUNCTION_CALL_TRACE;

    return iCalendar.uninit();
}

bool CalendarStorage::getAllItems( QList<Buteo::StorageItem*>& aItems )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Retrieving all calendar events and todo's" );

    KCalCore::Incidence::List incidences;

    if( !iCalendar.getAllIncidences( incidences ) ) {
        LOG_DEBUG( "Could not retrieve all calendar events and todo's" );
        return false;
    }

    retrieveItems( incidences, aItems );

    LOG_DEBUG( "Found" << aItems.count() << "items" );

    return true;
}

bool CalendarStorage::getAllItemIds( QList<QString>& aItemIds )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Retrieving all calendar events and todo's" );

    KCalCore::Incidence::List incidences;

    if( !iCalendar.getAllIncidences( incidences ) ) {
        LOG_DEBUG( "Could not retrieve all calendar events and todo's" );
        return false;
    }

    retrieveIds( incidences, aItemIds );

    LOG_DEBUG( "Found" << aItemIds.count() << "items" );

    return true;
}

bool CalendarStorage::getNewItems( QList<Buteo::StorageItem*>& aNewItems, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Retrieving new calendar events and todo's" );

    KCalCore::Incidence::List incidences;

    if( !iCalendar.getAllNew( incidences, normalizeTime( aTime ) ) ) {
        LOG_DEBUG( "Could not retrieve new calendar events and todo's" );
        return false;
    }

    retrieveItems( incidences, aNewItems );

    LOG_DEBUG( "Found" << aNewItems.count() << "new items" );

    return true;
}

bool CalendarStorage::getNewItemIds( QList<QString>& aNewItemIds, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Retrieving new calendar events and todo's" );

    KCalCore::Incidence::List incidences;

    if( !iCalendar.getAllNew( incidences, normalizeTime( aTime ) ) ) {
        LOG_DEBUG( "Could not retrieve new calendar events and todo's" );
        return false;
    }

    retrieveIds( incidences, aNewItemIds );

    LOG_DEBUG( "Found" << aNewItemIds.count() << "new items" );

    return true;
}

bool CalendarStorage::getModifiedItems( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Retrieving modified calendar events and todo's" );

    KCalCore::Incidence::List incidences;

    if( !iCalendar.getAllModified( incidences, normalizeTime( aTime ) ) ) {
        LOG_DEBUG( "Could not retrieve modified calendar events and todo's" );
        return false;
    }

    retrieveItems( incidences, aModifiedItems );

    LOG_DEBUG( "Found" << aModifiedItems.count() << "modified items" );

    return true;
}

bool CalendarStorage::getModifiedItemIds( QList<QString>& aModifiedItemIds, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Retrieving modified calendar events and todo's" );

    KCalCore::Incidence::List incidences;

    if( !iCalendar.getAllModified( incidences, normalizeTime( aTime ) ) ) {
        LOG_DEBUG( "Could not retrieve modified calendar events and todo's" );
        return false;
    }

    retrieveIds( incidences, aModifiedItemIds );

    LOG_DEBUG( "Found" << aModifiedItemIds.count() << "modified items" );

    return true;
}

bool CalendarStorage::getDeletedItemIds( QList<QString>& aDeletedItemIds, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Retrieving deleted calendar events and todo's" );

    KCalCore::Incidence::List incidences;

    if( !iCalendar.getAllDeleted( incidences, normalizeTime( aTime ) ) ) {
        LOG_DEBUG( "Could not retrieve deleted calendar events and todo's" );
        return false;
    }

    retrieveIds( incidences, aDeletedItemIds );

    LOG_DEBUG( "Found" << aDeletedItemIds.count() << "deleted items" );

    return true;
}

Buteo::StorageItem* CalendarStorage::newItem()
{
    FUNCTION_CALL_TRACE;

    return new SimpleItem();
}

QList<Buteo::StorageItem*> CalendarStorage::getItems( const QStringList& aItemIdList )
{
    FUNCTION_CALL_TRACE;

    KCalCore::Incidence::List incidences;
    KCalCore::Incidence::Ptr item;
    QList<Buteo::StorageItem*> items;
    QStringListIterator itr( aItemIdList );

    while( itr.hasNext() )
    {
        //TODO Does calendar backend support batch fetch, check!
        QString id = itr.next();
        item = iCalendar.getIncidence( id );
        if( item )
        {
            incidences.append( item );
        }
        else
        {
            LOG_WARNING("Could not find item " << id);
        }
    }

    retrieveItems( incidences, items );
 
    return items;
}

Buteo::StorageItem* CalendarStorage::getItem( const QString& aItemId )
{
    FUNCTION_CALL_TRACE;

    KCalCore::Incidence::Ptr item = iCalendar.getIncidence( aItemId );

    if( item ) {
        return retrieveItem( item );
    }
    else {
        LOG_WARNING( "Could not find item:" << aItemId );
        return NULL;
    }

}

CalendarStorage::OperationStatus CalendarStorage::addItem( Buteo::StorageItem& aItem )
{

    FUNCTION_CALL_TRACE;

    KCalCore::Incidence::Ptr item = generateIncidence( aItem );

    if( !item ) {
        LOG_WARNING( "Item has invalid format" );
        return STATUS_INVALID_FORMAT;
    }

    if( !iCalendar.addIncidence( item, iCommitNow ) ) {
        LOG_WARNING( "Could not add item" );
        // no need to delete item as item is owned by backend
        return STATUS_ERROR;
    }

    if (item->recurrenceId().isValid()) {  
	QString reccurId = QString(ID_SEPARATOR).append(item->recurrenceId().toString());    
       	aItem.setId( item->uid().append(reccurId) );
     } else {
        aItem.setId( item->uid() );
    }

    LOG_DEBUG( "Item successfully added:" << aItem.getId() );

    return STATUS_OK;

}

QList<CalendarStorage::OperationStatus> CalendarStorage::addItems( const QList<Buteo::StorageItem*>& aItems )
{
    FUNCTION_CALL_TRACE;

    QList<OperationStatus> results;

    // Disable auto commit as this is a batch add
    iCommitNow = false; 
    for( int i = 0; i < aItems.count(); ++i ) {
        results.append( addItem( *aItems[i] ) );
    }

    //Do a batch commit now
    if( iCalendar.commitChanges() )
    {
        LOG_DEBUG( "Items successfully added" );
    }
    iCommitNow = true; 

    return results;

}

CalendarStorage::OperationStatus CalendarStorage::modifyItem( Buteo::StorageItem& aItem )
{
    FUNCTION_CALL_TRACE;

    KCalCore::Incidence::Ptr item = generateIncidence( aItem );

    if( !item ) {
        LOG_WARNING( "Item has invalid format" );
        return STATUS_INVALID_FORMAT;
    }
    
    if( !iCalendar.modifyIncidence( item, aItem.getId(), iCommitNow ) ) {
        LOG_WARNING( "Could not replace item:" << aItem.getId() );
        // no need to delete item as item is owned by backend
        return STATUS_ERROR;
    }

    LOG_DEBUG( "Item successfully replaced:" << aItem.getId() );

    // modifyIncidence doesn't take ownership of the item, need to delete it.
    item.clear();

    return STATUS_OK;
}

QList<CalendarStorage::OperationStatus> CalendarStorage::modifyItems( const QList<Buteo::StorageItem*>& aItems )
{
    FUNCTION_CALL_TRACE;

    QList<OperationStatus> results;

    // Disable auto commit as this is a batch add
    iCommitNow = false; 
    for( int i = 0; i < aItems.count(); ++i ) {
        results.append( modifyItem( *aItems[i] ) );
    }

    //Do a batch commit now
    if( iCalendar.commitChanges() )
    {
        LOG_DEBUG( "Items successfully modified" );
    }
    iCommitNow = true; 

    return results;
}

CalendarStorage::OperationStatus CalendarStorage::deleteItem( const QString& aItemId )
{
    FUNCTION_CALL_TRACE;

    CalendarBackend::ErrorStatus error =  iCalendar.deleteIncidence( aItemId);
    CalendarStorage::OperationStatus status = mapErrorStatus(error);
    return status;
}

QList<CalendarStorage::OperationStatus> CalendarStorage::deleteItems( const QList<QString>& aItemIds )
{
    FUNCTION_CALL_TRACE;

    QList<OperationStatus> results;

    for( int i = 0; i < aItemIds.count(); ++i ) {
        results.append( deleteItem( aItemIds[i] ) );
    }

    return results;
}

KCalCore::Incidence::Ptr CalendarStorage::generateIncidence( Buteo::StorageItem& aItem )
{
	FUNCTION_CALL_TRACE;

    KCalCore::Incidence::Ptr incidence;
    QByteArray itemData;

    if( !aItem.read( 0, aItem.getSize(), itemData ) ) {
        LOG_WARNING( "Could not read item data" );
        return incidence;
    }

    QString data = QString::fromUtf8( itemData.data() );

    // we are getting a temporary incidence from the calendar
    if( iStorageType == VCALENDAR_FORMAT )
    {
        incidence = iCalendar.getIncidenceFromVcal( data );
    }
    else
    {
        incidence = iCalendar.getIncidenceFromIcal( data );
    }

    return incidence;
}

void CalendarStorage::retrieveItems( KCalCore::Incidence::List& aIncidences, QList<Buteo::StorageItem*>& aItems )
{
    FUNCTION_CALL_TRACE;

    for( int i = 0; i < aIncidences.count(); ++i ) {
        Buteo::StorageItem* item = retrieveItem( aIncidences[i] );
        aItems.append( item );
    }
}

Buteo::StorageItem* CalendarStorage::retrieveItem( KCalCore::Incidence::Ptr& aIncidence )
{
    FUNCTION_CALL_TRACE;

    QString data;

    if(iStorageType == VCALENDAR_FORMAT)
    {
        data = iCalendar.getVCalString( aIncidence );
    }
    else
    {
        data = iCalendar.getICalString( aIncidence);
    }

    Buteo::StorageItem* item = newItem();
    QString iId = aIncidence->uid();
    if (aIncidence->recurrenceId().isValid()) {  
	QString reccurId = QString(ID_SEPARATOR).append(aIncidence->recurrenceId().toString());    
       	iId.append(reccurId);
    }  
    item->setId(iId);
    item->write( 0, data.toUtf8() );
    item->setType(iProperties[STORAGE_DEFAULT_MIME_PROP]);

    return item;

}

void CalendarStorage::retrieveIds( KCalCore::Incidence::List& aIncidences, QList<QString>& aIds )
{
    FUNCTION_CALL_TRACE;

    for( int i = 0; i < aIncidences.count(); ++i ) {
        QString iID = aIncidences[i]->uid();
	if (aIncidences[i]->recurrenceId().isValid()) {  
	    QString reccurId = QString(ID_SEPARATOR).append(aIncidences[i]->recurrenceId().toString());    
            iID = iID.append(reccurId);
	}  
	aIds.append( iID );
    }

}

QDateTime CalendarStorage::normalizeTime( const QDateTime& aTime ) const
{
    FUNCTION_CALL_TRACE;

    QDateTime normTime = aTime;

    QTime time = aTime.time();
    time.setHMS( time.hour(), time.minute(), time.second(), 0 );

    normTime.setTime( time );

    normTime = normTime.toUTC();

    return normTime;
}

QByteArray CalendarStorage::getCtCaps( const QString& aFilename ) const
{
    FUNCTION_CALL_TRACE;

    QFile ctCapsFile( SyncMLConfig::getXmlDataPath() + aFilename  );
    QByteArray ctCaps;

    if( ctCapsFile.open(QIODevice::ReadOnly)) {
       ctCaps = ctCapsFile.readAll();
       ctCapsFile.close();
    } else {
        LOG_WARNING("Failed to open CTCaps file for calendar storage:" << aFilename );
    }

    return ctCaps;

}

CalendarStorage::OperationStatus CalendarStorage::mapErrorStatus\
        (const CalendarBackend::ErrorStatus &aCalenderError) const
{
    FUNCTION_CALL_TRACE;
    CalendarStorage::OperationStatus iStorageStatus = STATUS_OK;

    switch(aCalenderError) {
    case CalendarBackend::STATUS_OK:
            iStorageStatus = STATUS_OK;
            break;

    case CalendarBackend::STATUS_ITEM_DUPLICATE:
            iStorageStatus = STATUS_DUPLICATE;
            break;

    case CalendarBackend::STATUS_ITEM_NOT_FOUND:
            iStorageStatus = STATUS_NOT_FOUND;
            break;

    case CalendarBackend::STATUS_GENERIC_ERROR:
            iStorageStatus = STATUS_ERROR;
            break;

    default:
            iStorageStatus = STATUS_ERROR;
            break;
    }
    return iStorageStatus;
}
