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

#include "CalendarBackend.h"
#include <event.h>
#include <journal.h>
#include <LogMacros.h>
#include <QDir>
#include <QDebug>

CalendarBackend::CalendarBackend() : iCalendar( 0 ), iStorage( 0 )
{
	FUNCTION_CALL_TRACE;
}

CalendarBackend::~CalendarBackend()
{
	FUNCTION_CALL_TRACE;
}

namespace {
    mKCal::Notebook::Ptr defaultLocalCalendarNotebook(mKCal::ExtendedStorage::Ptr storage)
    {
        mKCal::Notebook::List notebooks = storage->notebooks();
        Q_FOREACH (const mKCal::Notebook::Ptr nb, notebooks) {
            if (nb->isMaster() && !nb->isShared() && nb->pluginName().isEmpty()) {
                // assume that this is the default local calendar notebook.
                return nb;
            }
        }
        LOG_WARNING("No default local calendar notebook found!");
        return mKCal::Notebook::Ptr();
    }
}

bool CalendarBackend::init(const QString& aUid)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Creating Default Maemo Storage");
    iCalendar = mKCal::ExtendedCalendar::Ptr( new mKCal::ExtendedCalendar( KDateTime::Spec::LocalZone( ) ));
    iStorage = iCalendar->defaultStorage( iCalendar );
    if (!iStorage->open()) {
        LOG_WARNING("Calendar storage open failed");
        iStorage.clear();
        iCalendar.clear();
        return false;
    }

    // If we have an Uid, we try to get the corresponding Notebook
    mKCal::Notebook::Ptr openedNb;
    if (!aUid.isEmpty()) {
        openedNb = iStorage->notebook(aUid);
        if (!openedNb) {
            LOG_WARNING("Invalid notebook UID specified:" << aUid << "- aborting sync");
            iStorage.clear();
            iCalendar.clear();
            return false;
        }
    } else {
        // otherwise, open the default local notebook.
        // Note: this is NOT the iStorage->defaultNotebook()
        // TODO: find out why we don't use the defaultNotebook here!
        openedNb = defaultLocalCalendarNotebook(iStorage);
        if (!openedNb) {
            LOG_WARNING("Unable to open the default local notebook for sync - aborting");
            iStorage.clear();
            iCalendar.clear();
            return false;
        }
    }

    if (!iStorage->loadNotebookIncidences(openedNb->uid())) {
        LOG_WARNING("Not able to initialize calendar");
        iStorage.clear();
        iCalendar.clear();
        return false;
    }

    iNotebookStr = openedNb->uid();
    LOG_DEBUG("Calendar initialized, opened notebook:" << openedNb->uid() << openedNb->name());
    return true;
}

bool CalendarBackend::uninit()
{
    FUNCTION_CALL_TRACE;

    if( iStorage ) {
        LOG_TRACE("Closing calendar storage...");
        iStorage->close();
        LOG_TRACE("Done");
        iStorage.clear();
    }

    if( iCalendar ) {
        LOG_TRACE("Closing calendar...");
        iCalendar->close();
        LOG_TRACE("Done");
        iCalendar.clear();
    }

    return true;
}

bool CalendarBackend::getAllIncidences( KCalCore::Incidence::List& aIncidences )
{
	FUNCTION_CALL_TRACE;

	if( !iStorage ) {
	    return false;
	}

	if( !iStorage->allIncidences( &aIncidences, iNotebookStr ) ) {
        LOG_WARNING("Error Retrieving ALL Incidences from the  Storage ");
        return false;
	}

    filterIncidences( aIncidences );
    return true;
}

void CalendarBackend::filterIncidences(KCalCore::Incidence::List& aList)
{
	FUNCTION_CALL_TRACE;
	QString event(INCIDENCE_TYPE_EVENT);
	QString todo(INCIDENCE_TYPE_TODO);

	for (int i = 0; i < aList.size(); ++i) {
	    KCalCore::Incidence::Ptr incidence = aList.at(i);
	    if ((incidence->type() != KCalCore::Incidence::TypeEvent) && (incidence->type() != KCalCore::Incidence::TypeTodo)) {
	        LOG_DEBUG("Removing incidence type" << incidence->typeStr());
                aList.remove( i, 1);
		incidence.clear();
	    }
        }
}

bool CalendarBackend::getAllNew( KCalCore::Incidence::List& aIncidences, const QDateTime& aTime )
{
    FUNCTION_CALL_TRACE;

    if( !iStorage ) {
        return false;
    }

    KDateTime kdt( aTime );

    if( !iStorage->insertedIncidences( &aIncidences ,kdt,iNotebookStr) ) {
        LOG_WARNING("Error Retrieving New Incidences from the Storage" );
        return false;
    }

    filterIncidences( aIncidences );
    return true;
}

bool CalendarBackend::getAllModified( KCalCore::Incidence::List& aIncidences, const QDateTime& aTime )
{
	FUNCTION_CALL_TRACE;

    if( !iStorage ) {
        return false;
    }

    KDateTime kdt( aTime );

    if( !iStorage->modifiedIncidences( &aIncidences, kdt,iNotebookStr ) ) {
        LOG_WARNING(" Error retrieving modified Incidences ");
        return false;
    }

    filterIncidences( aIncidences );
    return true;
}

bool CalendarBackend::getAllDeleted( KCalCore::Incidence::List& aIncidences, const QDateTime& aTime )
{
	FUNCTION_CALL_TRACE;

    if( !iStorage ) {
        return false;
    }

    KDateTime kdt( aTime );

    if( !iStorage->deletedIncidences( &aIncidences, kdt,iNotebookStr ) ) {
        LOG_WARNING(" Error retrieving deleted Incidences ");
        return false;
    }

    filterIncidences( aIncidences );
    return true;
}

KCalCore::Incidence::Ptr CalendarBackend::getIncidence( const QString& aUID )
{
    FUNCTION_CALL_TRACE;

    QStringList iDs = aUID.split(ID_SEPARATOR);
    KCalCore::Incidence::Ptr incidence;
    if (iDs.size() == 2) {
	   iStorage->load ( iDs.at(0), KDateTime::fromString(iDs.at(1)) );
	   incidence = iCalendar->incidence(iDs.at(0), KDateTime::fromString(iDs.at(1)));
    } else {
	   iStorage->load ( aUID );
	   incidence = iCalendar->incidence( aUID );
    }
    return incidence;
}

QString CalendarBackend::getVCalString(KCalCore::Incidence::Ptr aInci)
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( aInci );

    QString vcal;

    KCalCore::Incidence::Ptr temp = KCalCore::Incidence::Ptr ( aInci->clone() );
    if(temp) {
	KCalCore::Calendar::Ptr tempCalendar( new KCalCore::MemoryCalendar( KDateTime::UTC ) );
	tempCalendar->addIncidence(temp);
        KCalCore::VCalFormat vcf;
        vcal = vcf.toString(tempCalendar);
    }
    else {
    	LOG_WARNING("Error Cloning the Incidence for VCal String");
    }

    return vcal;
}

QString CalendarBackend::getICalString(KCalCore::Incidence::Ptr aInci)
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( aInci );

    KCalCore::Incidence::Ptr temp = KCalCore::Incidence::Ptr ( aInci->clone() );

    QString ical;
    if( temp ) {
	KCalCore::Calendar::Ptr tempCalendar( new KCalCore::MemoryCalendar( KDateTime::UTC ) );
	tempCalendar->addIncidence(temp);
	KCalCore::ICalFormat icf;
	ical = icf.toString(tempCalendar);
    }
    else {
    	LOG_WARNING("Error Cloning the Incidence for Ical String");
    }

    return ical;
}

void CalendarBackend::addIncidenceTimeZones(const KCalCore::Calendar::Ptr &aCalendar, const KCalCore::Incidence::Ptr &pInci)
{
    FUNCTION_CALL_TRACE;

    // adding possible timezone to iCalendar
    KCalCore::ICalTimeZones *tzlist = aCalendar->timeZones();
    if ( tzlist->count() > 0 ) {
        QString tz;
        if (pInci->dtStart().isValid()) {
            tz = pInci->dtStart().timeZone().name();
        }
        else {
            if ( pInci->type() == KCalCore::Incidence::TypeEvent ) {
                KCalCore::Event::Ptr event = pInci.staticCast<KCalCore::Event>();
                if (event->dtEnd().isValid()) {
                    tz = event->dtEnd().timeZone().name();
                }
            } else if( pInci->type() == KCalCore::Incidence::TypeTodo ) {
                KCalCore::Todo::Ptr todo = pInci.staticCast<KCalCore::Todo>();
                if ( todo->hasDueDate() ) {
                    tz = todo->dtDue( true ).timeZone().name();
                }
            } else {
                LOG_WARNING("Wrong incidence type" << pInci->type());
            }
        }
        if (!tz.isEmpty()) {
            KCalCore::ICalTimeZone zone = tzlist->zone(tz);
            if ( zone.isValid() ) {
                KCalCore::ICalTimeZones *zones = iCalendar->timeZones();
                zones->add( zone );
            }
        }
    }
}

KCalCore::Incidence::Ptr CalendarBackend::getIncidenceFromVcal( const QString& aVString )
{
    FUNCTION_CALL_TRACE;

    KCalCore::Incidence::Ptr pInci;

    KCalCore::Calendar::Ptr tempCalendar( new KCalCore::MemoryCalendar( KDateTime::Spec::LocalZone() ) );
    KCalCore::VCalFormat vcf;
    vcf.fromString(tempCalendar, aVString);
    KCalCore::Incidence::List lst = tempCalendar->rawIncidences();

    if(!lst.isEmpty()) {
        pInci = KCalCore::Incidence::Ptr ( lst[0]->clone() );
        addIncidenceTimeZones(tempCalendar, pInci);
    }
    else {
    	LOG_WARNING("VCal to Incidence Conversion Failed ");
    }
    return pInci;
}

KCalCore::Incidence::Ptr CalendarBackend::getIncidenceFromIcal( const QString& aIString )
{
	FUNCTION_CALL_TRACE;

    KCalCore::Incidence::Ptr pInci;

    KCalCore::Calendar::Ptr tempCalendar( new KCalCore::MemoryCalendar( KDateTime::Spec::LocalZone() ) );
    KCalCore::ICalFormat icf;
    icf.fromString(tempCalendar, aIString);
    KCalCore::Incidence::List lst = tempCalendar->rawIncidences();

    if(!lst.isEmpty()) {
        pInci = KCalCore::Incidence::Ptr ( lst[0]->clone() );
        addIncidenceTimeZones(tempCalendar, pInci);
    } else {
    	LOG_WARNING("ICal to Incidence Conversion Failed ");
    }

    return pInci;
}

bool CalendarBackend::addIncidence( KCalCore::Incidence::Ptr aInci, bool commitNow )
{
    FUNCTION_CALL_TRACE;

    if( !iCalendar || !iStorage ) {
        return false;
    }

    switch(aInci->type())
    {
        case KCalCore::Incidence::TypeEvent:
            {
                KCalCore::Event::Ptr event = aInci.staticCast<KCalCore::Event>();
                if(!iCalendar->addEvent(event, iNotebookStr))
                {
                    LOG_WARNING("Could not add event");
                    return false;
                }
            }
            break;
        case KCalCore::Incidence::TypeTodo:
            {
                KCalCore::Todo::Ptr todo = aInci.staticCast<KCalCore::Todo>();
                if(!iCalendar->addTodo(todo, iNotebookStr))
                {
                    LOG_WARNING("Could not add todo");
                    return false;
                }
            }
            break;
        default:
            LOG_WARNING("Could not add incidence, wrong type" << aInci->type());
            return false;
    }
    
    // if you add an incidence, that incidence will be owned by calendar
    // so no need to delete it. Committing for each modification, can cause performance
    // problems.

    if( commitNow )  {
        if( !iStorage->save() )
        {
            LOG_WARNING( "Could not commit changes to calendar");
            return false;
        }
        LOG_DEBUG( "Single incidence committed");
    }

    LOG_DEBUG("Added an item with UID : " << aInci->uid() << "Recurrence Id :" << aInci->recurrenceId().toString());

    return true;
}

bool CalendarBackend::commitChanges()
{
    FUNCTION_CALL_TRACE;
    bool changesCommitted = false;

    if( !iStorage )
    {
        LOG_WARNING("No calendar storage!");
    }
    else if( iStorage->save() )  {
        LOG_DEBUG( "Committed changes to calendar");
        changesCommitted = true;
    }
    else
    {
        LOG_DEBUG( "Could not commit changes to calendar");
    }
    return changesCommitted;
}

bool CalendarBackend::modifyIncidence( KCalCore::Incidence::Ptr aInci, const QString& aUID, bool commitNow )
{
	FUNCTION_CALL_TRACE;

    if( !iCalendar || !iStorage ) {
        return false;
    }

    KCalCore::Incidence::Ptr origInci = getIncidence ( aUID );

    if( !origInci ) {
        LOG_WARNING("Item with UID" << aUID << "does not exist. Cannot modify");
        return false;
    }

    if( !modifyIncidence( origInci, aInci ) ) {
        LOG_WARNING( "Could not make modifications to incidence" );
        return false;
    }

    if( commitNow )  {
        if( !iStorage->save() )
        {
            LOG_WARNING( "Could not commit changes to calendar");
            return false;
        }
        LOG_DEBUG( "Single incidence committed");
    }

    return true;
}

CalendarBackend::ErrorStatus CalendarBackend::deleteIncidence( const QString& aUID )
{
	FUNCTION_CALL_TRACE;
    CalendarBackend::ErrorStatus errorCode = CalendarBackend::STATUS_OK;

    if( !iCalendar || !iStorage ) {
        errorCode = CalendarBackend::STATUS_GENERIC_ERROR;
    }

    KCalCore::Incidence::Ptr incidence = getIncidence( aUID );
    
    if( !incidence ) {
        LOG_WARNING( "Could not find incidence to delete with UID" << aUID );
        errorCode = CalendarBackend::STATUS_ITEM_NOT_FOUND;
    }

    if( !iCalendar->deleteIncidence( incidence) )
    {
        LOG_WARNING( "Could not delete incidence with UID" << aUID );
        errorCode = CalendarBackend::STATUS_GENERIC_ERROR;
    }

    if( !iStorage->save() ) {
        LOG_WARNING( "Could not commit changes to calendar");
        errorCode =  CalendarBackend::STATUS_GENERIC_ERROR;
    }

    return errorCode;
}

bool CalendarBackend::modifyIncidence( KCalCore::Incidence::Ptr aIncidence, KCalCore::Incidence::Ptr aIncidenceData )
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( aIncidence );
    Q_ASSERT( aIncidenceData );

    // Save critical data from original item
    aIncidenceData->setUid( aIncidence->uid() );
    aIncidenceData->setCreated( aIncidence->created() );

    if( aIncidence->type() != aIncidenceData->type() ) {
        LOG_WARNING( "Expected incidence type" << aIncidence->typeStr() <<", got" << aIncidenceData->typeStr() );
        return false;
    }

    if( aIncidence->type() == KCalCore::Incidence::TypeEvent || aIncidence->type() == KCalCore::Incidence::TypeTodo )
    {
	KCalCore::IncidenceBase::Ptr inc = aIncidence;
	KCalCore::IncidenceBase::Ptr data = aIncidenceData;
        *inc = *data;    
    }
    else {
        LOG_WARNING( "Unsupported incidence type:" << aIncidence->typeStr() );
        return false;
    }

    iCalendar->setNotebook( aIncidence, iNotebookStr );

    return true;

}
