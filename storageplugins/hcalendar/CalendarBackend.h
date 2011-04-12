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
#include "memorycalendar.h"
#include "trackerstorage.h"
#include "notebook.h"
#include <sqlitestorage.h>
#include "vcalformat.h"
#include "icalformat.h"

static const QString ID_SEPARATOR("::");  
//! \brief Calendar implementation for synchronization
class CalendarBackend
{

public:
    //error status code return by calender backend.
    enum ErrorStatus
    {
        STATUS_GENERIC_ERROR = -3,           /*!< General error occurred during operation*/
        STATUS_ITEM_DUPLICATE = -2,       /*!< Operation was not performed as object was duplicate*/
        STATUS_ITEM_NOT_FOUND = -1,       /*!< Operation failed as object was not found*/
        STATUS_OK = 0                /*!< Operation was completed successfully*/
    };

    //! \brief constructor
    CalendarBackend();

    //! \brief destructor
    virtual ~CalendarBackend();

    //! \brief Initializes the CalendarBackend
    // \param strNotebookName Name of the notebook to use
    bool init( const QString& aNotebookName, const QString& aUid = "" );

    //! \brief Uninitializes the storage
    bool uninit();

    //! \brief returns all incidences inside this calendar
    // @param aIncidences List of incidences
    // @return True on success, otherwise false
    bool getAllIncidences( KCalCore::Incidence::List& aIncidences );

    //! \brief returns all new items after the date
    // @param aIncidences List of incidences
    // @param aTime Timestamp
    // @return True on success, otherwise false
    bool getAllNew( KCalCore::Incidence::List& aIncidences, const QDateTime& aTime );

    //! \brief returns all modified items after the date
    // @param aIncidences List of incidences
    // @param aTime Timestamp
    // @return True on success, otherwise false
    bool getAllModified( KCalCore::Incidence::List& aIncidences, const QDateTime& aTime );

    //! \brief returns all deleted items after the date
    // @param aIncidences List of incidences
    // @param aTime Timestamp
    // @return True on success, otherwise false
    bool getAllDeleted( KCalCore::Incidence::List& aIncidences, const QDateTime& aTime );

    //! \brief Get incidence based on uid.
    // Caller must not free the returned pointer.
    // \param aUID Item UID
    // \return The incidence (should not be freed by caller).
    KCalCore::Incidence::Ptr getIncidence( const QString& aUID );

    //! \brief returns VCalendar representation of incidence
    // \param pInci Incidence
    QString getVCalString( KCalCore::Incidence::Ptr aInci );

    //! \brief returns ICalendar representation of incidence
    // \param pInci Incidence
    QString getICalString( KCalCore::Incidence::Ptr aInci );

    //! \brief get Incidence from VCalendar string
    // Caller has to free the returned incidence after user.
    // \param aVString Incidence representation in VCalendar format.
    // \return Incidence pointer
    KCalCore::Incidence::Ptr getIncidenceFromVcal( const QString& aVString );

    //! \brief get Incidence from ICalendar string
    // Caller has to free the returned incidence after user.
    // \param aIString Incidence representation in ICalendar format.
    // \return Incidence pointer
    KCalCore::Incidence::Ptr getIncidenceFromIcal( const QString& aIString );

    //! \brief Add the incidence to calendar
    //
    // Duplicate checking will be done if id the of item is not empty.
    // If an entry exists, it will be overwritten.
    // The uid of incidence will be updated.
    // \param aInci Incidence to be added.
    // \param commitNow - indicates if we have to commit to the backend immediately
    // \return true if addition is success, otherwise false
    bool addIncidence( KCalCore::Incidence::Ptr aInci, bool commitNow = true );

    //! \brief Tell Calendar to commit changes to their db
    /// \return true if committed succesfully, false otherwise
    bool commitChanges();

    //! \brief Modify the incidence in calendar
    //
    // if there is no incidence with old id exists, a new incidence
    // will be created.
    // The uid of incidence will be updated.
    // \param aInci Incidence to be modified
    // \param aUID UID of item
    // \return true if modification was success, otherwise false
    bool modifyIncidence( KCalCore::Incidence::Ptr aInci, const QString& aUID, bool commitNow = true );

    //! \brief delete the incidence
    // \param aUID id of the incidence to be deleted
    // \return errorCode of the operation as status.
    ErrorStatus deleteIncidence( const QString& aUID);

private:
    bool modifyIncidence( KCalCore::Incidence::Ptr aIncidence, KCalCore::Incidence::Ptr aIncidenceData );

    void filterIncidences( KCalCore::Incidence::List& aList );

    void addIncidenceTimeZones(const KCalCore::Calendar::Ptr &aCalendar, const KCalCore::Incidence::Ptr &pInci);

    QString                 iNotebookStr;
    mKCal::ExtendedCalendar::Ptr  iCalendar;
    mKCal::ExtendedStorage::Ptr   iStorage;

};


#endif //CALENDARBACKEND_H_490498898043897984389983478
