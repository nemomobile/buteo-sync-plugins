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

#ifndef DEFINITIONS_HEADER_4089094908098437549308790580985
#define DEFINITIONS_HEADER_4089094908098437549308790580985

#include <QDate>
#include <QString>

const QString DEFAULT_NOTEBOOK_NAME = "myNotebook";
const QString BACKEND_DATABASE      = "organizer.db";

//The storage expect this property from outside. If no name is specified, it
// will use DEFAULT_NOTEBOOK_NAME.
const QString NOTEBOOKNAME = "Notebook Name";

//The storage expect this property from outside. If no format is specified
// the storage will use CALENDAR_FORMAT_VCAL.
const QString CALENDAR_FORMAT        = "Calendar Format";

//Possible values of calendar format are CALENDAR_FORMAT_VCAL
// and CALENDAR_FORMAT_ICAL
const QString CALENDAR_FORMAT_VCAL = "vcalendar";
const QString CALENDAR_FORMAT_ICAL = "icalendar";

//The incidences before this date wont be considered for sync
const QDate OLDESTDATE(1970,01,01);

// Calendar incidence types. Needed for filtering purposes.
#define CalendarIncidenceType QString
const CalendarIncidenceType INCIDENDE_TYPE_ALL     = "";
const CalendarIncidenceType INCIDENCE_TYPE_JOURNAL = "Journal";
const CalendarIncidenceType INCIDENCE_TYPE_EVENT   = "Event";
const CalendarIncidenceType INCIDENCE_TYPE_TODO    = "Todo";

#endif //DEFINITIONS_HEADER_4089094908098437549308790580985
