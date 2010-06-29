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

class QDateTime;

namespace Buteo {
    class StorageItem;
}

namespace KCal {
    class ExtendedCalendar;
    class ExtendedStorage;
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
    bool init( const QString& aNotebookName, const QString &aMimeType );

    /*! \brief Uninitializes backend
     *
     * @return True on success, otherwise false
     */
    bool uninit();

    bool getAllNotes( QList<Buteo::StorageItem*>& aItems );

    bool getAllNoteIds( QList<QString>& aIds );

    bool getNewNotes( QList<Buteo::StorageItem*>& aNewItems, const QDateTime& aTime );

    bool getNewNoteIds( QList<QString>& aNewIds, const QDateTime& aTime );

    bool getModifiedNotes( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime );

    bool getModifiedNoteIds( QList<QString>& aModifiedIds, const QDateTime& aTime );

    bool getDeletedNoteIds( QList<QString>& aDeletedIds, const QDateTime& aTime );

    Buteo::StorageItem* newItem();

    Buteo::StorageItem* getItem( const QString& aItemId );

    bool addNote( Buteo::StorageItem& aItem );

    bool modifyNote( Buteo::StorageItem& aItem );

    bool deleteNote( const QString& aId );

protected:

private:

    void retrieveNoteItems( KCal::Incidence::List& aIncidences, QList<Buteo::StorageItem*>& aItems );

    void retrieveNoteIds( KCal::Incidence::List& aIncidences, QList<QString>& aIds );

    void filterIncidences( KCal::Incidence::List& aIncidences );


    QString                 iNotebookName;
    QString                 iMimeType;

    KCal::ExtendedCalendar*    iCalendar;
    KCal::ExtendedStorage*    iStorage;

};

#endif  //  NOTESBACKEND_H
