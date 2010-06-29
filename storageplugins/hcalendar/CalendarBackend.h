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

#ifndef CALENDARBACKEND_H_490498898043897984389983478
#define CALENDARBACKEND_H_490498898043897984389983478

#include <QString>

#include "definitions.h"

//calendar related includes
#include "extendedcalendar.h"
#include "trackerstorage.h"
#include "notebook.h"
#include <sqlitestorage.h>
#include "vcalformat.h"
#include "icalformat.h"

/// \brief Calendar implementation for synchronization
class CalendarBackend
{

public:
    /// \brief constructor
    CalendarBackend();

    /// \brief destructor
    virtual ~CalendarBackend();

    /// \brief Initializes the CalendarBackend
    /// \param strNotebookName Name of the notebook to use
    bool init( const QString& aNotebookName );

    /// \brief Uninitializes the storage
    bool uninit();

    /// \brief returns all incidences inside this calendar
    /// @param aIncidences List of incidences
    /// @return True on success, otherwise false
    bool getAllIncidences( KCal::Incidence::List& aIncidences );

    /// \brief returns all new items after the date
    /// @param aIncidences List of incidences
    /// @param aTime Timestamp
    /// @return True on success, otherwise false
    bool getAllNew( KCal::Incidence::List& aIncidences, const QDateTime& aTime );

    /// \brief returns all modified items after the date
    /// @param aIncidences List of incidences
    /// @param aTime Timestamp
    /// @return True on success, otherwise false
    bool getAllModified( KCal::Incidence::List& aIncidences, const QDateTime& aTime );

    /// \brief returns all deleted items after the date
    /// @param aIncidences List of incidences
    /// @param aTime Timestamp
    /// @return True on success, otherwise false
    bool getAllDeleted( KCal::Incidence::List& aIncidences, const QDateTime& aTime );

    /// \brief Get incidence based on uid.
    /// Caller must not free the returned pointer.
    /// \param aUID Item UID
    /// \return The incidence (should not be freed by caller).
    KCal::Incidence* getIncidence( const QString& aUID );

    /// \brief returns VCalendar representation of incidence
    /// \param pInci Incidence
    QString getVCalString( KCal::Incidence* aInci );

    /// \brief returns ICalendar representation of incidence
    /// \param pInci Incidence
    QString getICalString( KCal::Incidence* aInci );

    /// \brief get Incidence from VCalendar string
    /// Caller has to free the returned incidence after user.
    /// \param aVString Incidence representation in VCalendar format.
    /// \return Incidence pointer
    KCal::Incidence* getIncidenceFromVcal( const QString& aVString );

    /// \brief get Incidence from ICalendar string
    /// Caller has to free the returned incidence after user.
    /// \param aIString Incidence representation in ICalendar format.
    /// \return Incidence pointer
    KCal::Incidence* getIncidenceFromIcal( const QString& aIString );

    /// \brief Add the incidence to calendar
    ///
    /// Duplicate checking will be done if id the of item is not empty.
    /// If an entry exists, it will be overwritten.
    /// The uid of incidence will be updated.
    /// \param aInci Incidence to be added.
    /// \return true if addition is success, otherwise false
    bool addIncidence( KCal::Incidence* aInci, bool commitNow = true );

    /// \bries Tell Calendar to commit changes to their db
    /// \return true if committed succesfully, false otherwise
    bool commitChanges();

    /// \brief Modify the incidence in calendar
    ///
    /// if there is no incidence with old id exists, a new incidence
    /// will be created.
    /// The uid of incidence will be updated.
    /// \param aInci Incidence to be modified
    /// \param aUID UID of item
    /// \return true if modification was success, otherwise false
    bool modifyIncidence( KCal::Incidence* aInci, const QString& aUID, bool commitNow = true );

    /// \brief delete the incidence
    /// \param aUID id of the incidence to be deleted
    /// \return true if deletion is success, otherwise false
    bool deleteIncidence( const QString& aUID );

    /// \brief prints contents of calendar. only for debuuging purpose
    void printCalendar();

private:

    bool modifyIncidence( KCal::Incidence* aIncidence, KCal::Incidence* aIncidenceData );

    void filterIncidences( KCal::Incidence::List& aList );


    QString                 iNotebookStr;
    KCal::ExtendedCalendar*    iCalendar;
    KCal::ExtendedStorage*   iStorage;

};


#endif //CALENDARBACKEND_H_490498898043897984389983478
