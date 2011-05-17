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
#ifndef NOTESBACKEND_H
#define NOTESBACKEND_H

#include <QString>
#include <incidence.h>
#include <extendedcalendar.h>
#include <extendedstorage.h>

class QDateTime;

namespace Buteo {
    class StorageItem;
}

/*! \brief Notes Calendar backend proxy
 *
 */
class NotesBackend
{
public:

    /*! \brief Constructor
     *
     */
    NotesBackend();

    /*! \brief Destructor
     *
     */
    virtual ~NotesBackend();

    /*! \brief Initializes backend
     *
     * @return True on success, otherwise false
     */
    bool init( const QString& aNotebookName, const QString& aUid, const QString &aMimeType );

    /*! \brief Uninitializes backend
     *
     * @return True on success, otherwise false
     */
    bool uninit();

    /*! \brief retrieves all Notes from the backend
     *
     * @param  aItems Output Parameter - List of Buteo::StorageItems retrieved from Backend.
     * @return True on success, otherwise false
     */
    bool getAllNotes( QList<Buteo::StorageItem*>& aItems );

    /*! \brief gets are note ids from the backend
     *
     * @param aIds - list of notes ids
     * @return True on success, otherwise false
     */
    bool getAllNoteIds( QList<QString>& aIds );

    /*! \brief get all new notes from a timestamp
     *
     * @param  aNewItems List of new items.(output parameter)
     * @param  aTime - time from which to retrieve the notes
     * @return True on success, otherwise false
     */
    bool getNewNotes( QList<Buteo::StorageItem*>& aNewItems, const QDateTime& aTime );

    /*! \brief get all new note ids from a timestamp
     *
     * @param  aNewIds List of new ids (output parameter)
     * @param  aTime - time from which to retrieve the notes
     * @return True on success, otherwise false
     */
    bool getNewNoteIds( QList<QString>& aNewIds, const QDateTime& aTime );

    /*! \brief get all modified notes from the backend
     *
     * @param aModifiedItems - list of modified items (output parameter)
     * @param aTime -    timestamp from which the modified notes are needed.
     * @return True on success, otherwise false
     */
    bool getModifiedNotes( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime );

    /*! \brief get all modified notes ids from the backend
     *
     * @param aModifiedIds - list of modified ids (output parameter)
     * @param aTime -    timestamp from which the modified notes ids are needed.
     * @return True on success, otherwise false
     */
    bool getModifiedNoteIds( QList<QString>& aModifiedIds, const QDateTime& aTime );

    /*! \brief gets all deleted note ids
     *
     * @param aDeletedIds - deleted ids.
     * @param aTime - timestamp from which the deleted ids are needed.
     * @return True on success, otherwise false
     */
    bool getDeletedNoteIds( QList<QString>& aDeletedIds, const QDateTime& aTime );

    /*! \brief fetch a new StorageItem
     *
     * @return pointer to the newly created StorageItem
     */
    Buteo::StorageItem* newItem();

    /*! \brief get an item
     *
     * @param aItemId - id of the item to get
     * @return pointer to the StorageItem
     */
    Buteo::StorageItem* getItem( const QString& aItemId );

    /*! \brief Uninitializes backend
     *
     * @param aItem - item to add
     * @param aCommitNow - if true persist db changes, if false do this later
     * @return True on success, otherwise false
     */
    bool addNote( Buteo::StorageItem& aItem, bool commitNow );

    /*! \brief Uninitializes backend
     *
     * @param aItem - item to modify
     * @param aCommitNow - if true persist db changes, if false do this later
     * @return True on success, otherwise false
     */
    bool modifyNote( Buteo::StorageItem& aItem, bool commitNow );

    /*! \brief Uninitializes backend
     *
     * @param aId - id of item to delete
     * @param aCommitNow - if true persist db changes, if false do this later
     * @return True on success, otherwise false
     */
    bool deleteNote( const QString& aId, bool commitNow );

    /*! \brief Persist notes db
     *
     * @return True on success, otherwise false
     */
    bool commitChanges();

protected:

private:

    void retrieveNoteItems( KCalCore::Incidence::List& aIncidences, QList<Buteo::StorageItem*>& aItems );

    void retrieveNoteIds( KCalCore::Incidence::List& aIncidences, QList<QString>& aIds );

    void filterIncidences( KCalCore::Incidence::List& aIncidences );


    QString                 iNotebookName;
    QString                 iMimeType;

    mKCal::ExtendedCalendar::Ptr    iCalendar;
    mKCal::ExtendedStorage::Ptr    iStorage;

};

#endif  //  NOTESBACKEND_H
