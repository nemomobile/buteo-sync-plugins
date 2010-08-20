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

#include "NotesBackend.h"

#include <extendedcalendar.h>
#include <sqlitestorage.h>
#include <QDir>

#include <LogMacros.h>

#include "SimpleItem.h"

// @todo: handle unicode notes better. For example S60 seems to send only ascii.
//        Ovi.com seems to send latin-1 in base64-encoded form. UTF-8 really should
//        be preferred here, but how would we know which format is given to us as
//        latin-1 and utf-8 are not compatible?

static const QString INCIDENCE_TYPE_JOURNAL( "Journal" );

NotesBackend::NotesBackend() : iCalendar( 0 ), iStorage( 0 )
{
    FUNCTION_CALL_TRACE;
}

NotesBackend::~NotesBackend()
{
    FUNCTION_CALL_TRACE;
}

bool NotesBackend::init( const QString& aNotebookName,
                         const QString &aMimeType )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Notes backend using notebook" << aNotebookName );

    iNotebookName = aNotebookName;
    iMimeType = aMimeType;

    iCalendar = new KCal::ExtendedCalendar( QLatin1String( "UTC" ) );

    LOG_DEBUG("Creating Default Maemo Storage for Notes");
    iStorage = iCalendar->defaultStorage();

    bool opened = iStorage->open();

    bool loaded = false;
    if (opened)
    {
        LOG_TRACE("Calendar storage opened");
        loaded = iStorage->load();
        if (!loaded)
        {
            LOG_WARNING("Failed to load calendar");
        } // no else
    } else {
    	LOG_TRACE("Calendar storage open failed");
    }

    // Use default notebook to sync , we use calendar id now instead of name
    // This functionality is temporary
    bool hasdefNb = false;
    KCal::Notebook *defaultNb = iStorage->defaultNotebook();
    if (defaultNb){
	    iNotebookName = defaultNb->uid();
	    LOG_TRACE ("Default NoteBook UID" << iNotebookName);
	    hasdefNb = true;
    }
    
    if (opened && loaded && hasdefNb)
    {
        LOG_DEBUG("Calendar initialized");
        return true;
    }
    else
    {
        LOG_WARNING("Not able to initialize calendar");

        delete iStorage;
        iStorage = 0;

        LOG_TRACE("Storage deleted");

        delete iCalendar;
        iCalendar = 0;

        LOG_TRACE("Calendar deleted");

        return false;
    }

}


bool NotesBackend::uninit()
{
    FUNCTION_CALL_TRACE;

    if( iStorage ) {
        iStorage->close();
        delete iStorage;
        iStorage = NULL;
    }

    if( iCalendar ) {
        iCalendar->close();
        delete iCalendar;
        iCalendar = NULL;
    }

    return true;
}

bool NotesBackend::getAllNotes( QList<Buteo::StorageItem*>& aItems )
{

    FUNCTION_CALL_TRACE;

    KCal::Incidence::List incidences;
    incidences.setAutoDelete(true);

    if( !iStorage->allIncidences( &incidences, iNotebookName ) ) {
        LOG_WARNING( "Could not retrieve all notes" );
        return false;
    }

    retrieveNoteItems( incidences, aItems );

    return true;

}

bool NotesBackend::getAllNoteIds( QList<QString>& aItemIds )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence::List incidences;
    incidences.setAutoDelete(true);

    if( !iStorage->allIncidences( &incidences, iNotebookName ) ) {
        LOG_WARNING( "Could not retrieve all notes" );
        return false;
    }

    retrieveNoteIds( incidences, aItemIds );

    return true;
}

bool NotesBackend::getNewNotes( QList<Buteo::StorageItem*>& aNewItems, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence::List incidences;
    incidences.setAutoDelete(true);
    KDateTime kdt( aTime );

    if( !iStorage->insertedIncidences( &incidences, kdt, iNotebookName ) ) {
        LOG_WARNING( "Could not retrieve new notes" );
        return false;
    }

    retrieveNoteItems( incidences, aNewItems );

    return true;
}

bool NotesBackend::getNewNoteIds( QList<QString>& aNewItemIds, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence::List incidences;
    incidences.setAutoDelete(true);
    KDateTime kdt( aTime );

    if( !iStorage->insertedIncidences( &incidences, kdt, iNotebookName ) ) {
        LOG_WARNING( "Could not retrieve new notes" );
        return false;
    }

    retrieveNoteIds( incidences, aNewItemIds );

    return true;
}

bool NotesBackend::getModifiedNotes( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence::List incidences;
    incidences.setAutoDelete(true);
    KDateTime kdt( aTime );

    if( !iStorage->modifiedIncidences( &incidences, kdt, iNotebookName ) ) {
        LOG_WARNING( "Could not retrieve modified notes" );
        return false;
    }

    retrieveNoteItems( incidences, aModifiedItems );

    return true;
}

bool NotesBackend::getModifiedNoteIds( QList<QString>& aModifiedItemIds, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence::List incidences;
    incidences.setAutoDelete(true);
    KDateTime kdt( aTime );

    if( !iStorage->modifiedIncidences( &incidences, kdt, iNotebookName ) ) {
        LOG_WARNING( "Could not retrieve modified notes" );
        return false;
    }

    retrieveNoteIds( incidences, aModifiedItemIds );

    return true;
}

bool NotesBackend::getDeletedNoteIds( QList<QString>& aDeletedItemIds, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence::List incidences;
    incidences.setAutoDelete(true);
    KDateTime kdt( aTime );

    if( !iStorage->deletedIncidences( &incidences, kdt, iNotebookName ) ) {
        LOG_WARNING( "Could not retrieve modified notes" );
        return false;
    }

    retrieveNoteIds( incidences, aDeletedItemIds );

    return true;

}

Buteo::StorageItem* NotesBackend::newItem()
{
    FUNCTION_CALL_TRACE;

    return new SimpleItem;
}

Buteo::StorageItem* NotesBackend::getItem( const QString& aItemId )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence* item = iCalendar->incidence( aItemId );

    if( !item ) {
        LOG_WARNING( "Could not find item:" << aItemId );
        return NULL;
    }

    Buteo::StorageItem* storageItem = newItem();
    storageItem->setId( item->uid() );
    storageItem->setType(iMimeType);
    storageItem->write( 0, item->description().toUtf8() );

    return storageItem;
}

bool NotesBackend::addNote( Buteo::StorageItem& aItem )
{
    FUNCTION_CALL_TRACE;

    QByteArray data;

    if( !aItem.read( 0, aItem.getSize(), data ) ) {
        LOG_WARNING( "Reading item data failed" );
        return false;
    }

    KCal::Journal* journal = new KCal::Journal();

    QString description = QString::fromUtf8( data.constData() );

    // Trim the description to make sure that it does not include for example
    // line feeds at the end.
    journal->setDescription( description.trimmed() );

    // addIncidence() takes ownership of journal -> we cannot delete it

    if( !iCalendar->addIncidence( journal ) ) {
        LOG_WARNING( "Could not add note to calendar" );
        delete journal;
        journal = 0;
        return false;
    }

    iCalendar->setNotebook( journal, iNotebookName );
    if( !iStorage->save() )  {
        LOG_WARNING( "Could not save note to storage" );
        return false;
    }

    QString id = journal->uid();

    LOG_DEBUG( "New note added, id:" << id );

    aItem.setId( id );

    return true;

}

bool NotesBackend::modifyNote( Buteo::StorageItem& aItem )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence* item = iCalendar->incidence( aItem.getId() );

    if( !item ) {
        LOG_WARNING( "Could not find item to be modified:" << aItem.getId() );
        return false;
    }

    QByteArray data;

    if( !aItem.read( 0, aItem.getSize(), data ) ) {
        LOG_WARNING( "Reading item data failed:" << aItem.getId() );
        return false;
    }

    QString description = QString::fromAscii( data.constData() );

    // Trim the description to make sure that it does not include for example
    // line feeds at the end.
    item->setDescription( description.trimmed() );

    if( !iStorage->save() )  {
        LOG_WARNING( "Could not save modification to storage" );
        return false;
    }

    LOG_DEBUG( "Note modified, id:" << aItem.getId() );

    return true;
}

bool NotesBackend::deleteNote( const QString& aId )
{
    FUNCTION_CALL_TRACE;

    KCal::Incidence* journal = iCalendar->incidence( aId );

    if( !journal ) {
        LOG_WARNING( "Could not find item to be deleted:" << aId );
        return false;
    }

    if( !iCalendar->deleteIncidence( journal ) ) {
        LOG_WARNING( "Could not delete note:" << aId );
        return false;
    }

    if( !iStorage->save() ) {
        LOG_WARNING( "Could not commit deleting of note:" << aId );
        return false;
    }

    return true;
}


void NotesBackend::retrieveNoteItems( KCal::Incidence::List& aIncidences, QList<Buteo::StorageItem*>& aItems )
{
    FUNCTION_CALL_TRACE;

    filterIncidences( aIncidences );

    for( int i = 0; i < aIncidences.count(); ++i ) {
        Buteo::StorageItem* item = newItem();
        item->setId( aIncidences[i]->uid() );
        item->setType(iMimeType);
        item->write( 0, aIncidences[i]->description().toUtf8() );
        aItems.append( item );
    }

}

void NotesBackend::retrieveNoteIds( KCal::Incidence::List& aIncidences, QList<QString>& aIds )
{
    FUNCTION_CALL_TRACE;

    filterIncidences( aIncidences );

    for( int i = 0; i < aIncidences.count(); ++i ) {
        aIds.append( aIncidences[i]->uid() );
    }

}

void NotesBackend::filterIncidences( KCal::Incidence::List& aIncidences )
{
    FUNCTION_CALL_TRACE;

    QString journal( INCIDENCE_TYPE_JOURNAL );

    int i = 0;
    while( i < aIncidences.count() ) {
        KCal::Incidence* incidence = aIncidences[i];

        if( incidence->type() != journal ) {
            aIncidences.removeRef( incidence );
            // Removed incidences are deleted by the list that is in
            // auto delete mode.
        }
        else {
            ++i;
        }
    }

}
