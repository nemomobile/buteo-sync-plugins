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

    iCalendar = mKCal::ExtendedCalendar::Ptr( new mKCal::ExtendedCalendar( KDateTime::Spec::LocalZone( ) ));

    LOG_DEBUG("Creating Default Maemo Storage for Notes");
    iStorage = iCalendar->defaultStorage( iCalendar );

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
    mKCal::Notebook::Ptr defaultNb = iStorage->defaultNotebook();
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

        iStorage.clear();

        LOG_TRACE("Storage deleted");

        iCalendar.clear();

        LOG_TRACE("Calendar deleted");

        return false;
    }

}


bool NotesBackend::uninit()
{
    FUNCTION_CALL_TRACE;

    if( iStorage ) {
        iStorage->close();
        iStorage.clear();
    }

    if( iCalendar ) {
        iCalendar->close();
        iCalendar.clear();
    }

    return true;
}

bool NotesBackend::getAllNotes( QList<Buteo::StorageItem*>& aItems )
{

    FUNCTION_CALL_TRACE;

    KCalCore::Incidence::List incidences;

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

    KCalCore::Incidence::List incidences;

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

    KCalCore::Incidence::List incidences;
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

    KCalCore::Incidence::List incidences;
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

    KCalCore::Incidence::List incidences;
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

    KCalCore::Incidence::List incidences;
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

    KCalCore::Incidence::List incidences;
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

    iStorage->load ( aItemId );
    KCalCore::Incidence::Ptr item = iCalendar->incidence( aItemId );

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

bool NotesBackend::addNote( Buteo::StorageItem& aItem, bool aCommitNow )
{
    FUNCTION_CALL_TRACE;

    QByteArray data;

    if( !aItem.read( 0, aItem.getSize(), data ) ) {
        LOG_WARNING( "Reading item data failed" );
        return false;
    }

    KCalCore::Journal::Ptr journal ;
    journal = KCalCore::Journal::Ptr( new KCalCore::Journal() );

    QString description = QString::fromUtf8( data.constData() );

    // Trim the description to make sure that it does not include for example
    // line feeds at the end.
    journal->setDescription( description.trimmed() );

    // addIncidence() takes ownership of journal -> we cannot delete it

    if( !iCalendar->addIncidence( journal ) ) {
        LOG_WARNING( "Could not add note to calendar" );
        journal.clear();
        return false;
    }

    iCalendar->setNotebook( journal, iNotebookName );

    QString id = journal->uid();

    LOG_DEBUG( "New note added, id:" << id );

    aItem.setId( id );

    if( aCommitNow )
    {
        if( !commitChanges() )
        {
            return false;
        }
    }

    return true;

}

bool NotesBackend::modifyNote( Buteo::StorageItem& aItem, bool aCommitNow )
{
    FUNCTION_CALL_TRACE;

    iStorage->load ( aItem.getId() );
    KCalCore::Incidence::Ptr item = iCalendar->incidence( aItem.getId() );

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

    if( aCommitNow )
    {
        if( !commitChanges() )
        {
            return false;
        }
    }

    LOG_DEBUG( "Note modified, id:" << aItem.getId() );

    return true;
}

bool NotesBackend::deleteNote( const QString& aId, bool aCommitNow )
{
    FUNCTION_CALL_TRACE;

    iStorage->load ( aId );
    KCalCore::Incidence::Ptr journal = iCalendar->incidence( aId );

    if( !journal ) {
        LOG_WARNING( "Could not find item to be deleted:" << aId );
        return false;
    }

    if( !iCalendar->deleteIncidence( journal ) ) {
        LOG_WARNING( "Could not delete note:" << aId );
        return false;
    }

    if( aCommitNow )
    {
        if( !commitChanges() )
        {
            return false;
        }
    }

    return true;
}


bool NotesBackend::commitChanges()
{
    bool saved = false;

    if( iStorage && iStorage->save() )
    {
        saved = true;
    }
    else
    {
        LOG_CRITICAL("Couldn't save to storage");
    }
    return saved;
}

void NotesBackend::retrieveNoteItems( KCalCore::Incidence::List& aIncidences, QList<Buteo::StorageItem*>& aItems )
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

void NotesBackend::retrieveNoteIds( KCalCore::Incidence::List& aIncidences, QList<QString>& aIds )
{
    FUNCTION_CALL_TRACE;

    filterIncidences( aIncidences );

    for( int i = 0; i < aIncidences.count(); ++i ) {
        aIds.append( aIncidences[i]->uid() );
    }

}

void NotesBackend::filterIncidences( KCalCore::Incidence::List& aIncidences )
{
    FUNCTION_CALL_TRACE;

    QString journal( INCIDENCE_TYPE_JOURNAL );

    int i = 0;
    while( i < aIncidences.count() ) {
        KCalCore::Incidence::Ptr incidence = aIncidences[i];

        if( incidence->type() != KCalCore::Incidence::TypeJournal ) {
            aIncidences.remove( i, 1 );
	    incidence.clear();
        }
        else {
            ++i;
        }
    }

}
