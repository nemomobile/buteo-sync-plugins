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

bool CalendarBackend::init( const QString& aNotebookName )
{
	FUNCTION_CALL_TRACE;

    if( aNotebookName.isEmpty() )
    {
    	LOG_DEBUG("NoteBook Name to Sync is expected. It Cannot be Empty");
        return false;
    }

    iNotebookStr = aNotebookName;

    iCalendar = new KCal::ExtendedCalendar(KDateTime::Spec::LocalZone());

    LOG_DEBUG("Creating Default Maemo Storage");
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
	    iNotebookStr = defaultNb->uid();
	    LOG_TRACE ("Default NoteBook UID" << iNotebookStr);
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

bool CalendarBackend::uninit()
{
    FUNCTION_CALL_TRACE;

    if( iStorage ) {
        LOG_TRACE("Closing calendar storage...");
        iStorage->close();
        LOG_TRACE("Done");
        delete iStorage;
        iStorage = NULL;
    }

    if( iCalendar ) {
        LOG_TRACE("Closing calendar...");
        iCalendar->close();
        LOG_TRACE("Done");
        delete iCalendar;
        iCalendar = NULL;
    }

    return true;
}

bool CalendarBackend::getAllIncidences( KCal::Incidence::List& aIncidences )
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

bool CalendarBackend::getAllNew( KCal::Incidence::List& aIncidences, const QDateTime& aTime )
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

bool CalendarBackend::getAllModified( KCal::Incidence::List& aIncidences, const QDateTime& aTime )
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

bool CalendarBackend::getAllDeleted( KCal::Incidence::List& aIncidences, const QDateTime& aTime )
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

KCal::Incidence* CalendarBackend::getIncidence( const QString& aUID )
{
    FUNCTION_CALL_TRACE;

    return iCalendar->incidence( aUID );
}

QString CalendarBackend::getVCalString(KCal::Incidence* aInci)
{
	FUNCTION_CALL_TRACE;

	Q_ASSERT( aInci );

	QString vcal;

    //If you add an incidence to a calendar, it will be owned by calendar
    //So it is safe to pass only a clone.
    KCal::Incidence* temp = aInci->clone();

    if(temp) {
    	KCal::CalendarLocal tempCalendar(QLatin1String("UTC"));
    	tempCalendar.addIncidence(temp);
        KCal::VCalFormat vcf;
        vcal = vcf.toString(&tempCalendar);
    }
    else {
    	LOG_WARNING("Error Cloning the Incidence for VCal String");
    }

    return vcal;
}

QString CalendarBackend::getICalString(KCal::Incidence* aInci)
{
	FUNCTION_CALL_TRACE;

	Q_ASSERT( aInci );

	QString ical;

    //If you add an incidence to a calendar, it will be owned by calendar
    //So it is safe to pass only a clone.
    KCal::Incidence* temp = aInci->clone();

    if( temp ) {
		KCal::CalendarLocal tempCalendar(QLatin1String("UTC"));
		tempCalendar.addIncidence(temp);
	    KCal::ICalFormat icf;
	    ical = icf.toString(&tempCalendar);
    }
    else {
    	LOG_WARNING("Error Cloning the Incidence for Ical String");
    }

    return ical;
}

KCal::Incidence* CalendarBackend::getIncidenceFromVcal( const QString& aVString )
{
	FUNCTION_CALL_TRACE;

	KCal::Incidence* pInci = NULL;

    KCal::CalendarLocal tempCalendar(KDateTime::Spec::LocalZone());
    KCal::VCalFormat vcf;
    vcf.fromString(&tempCalendar, aVString);
    KCal::Incidence::List lst = tempCalendar.rawIncidences();

    if(!lst.isEmpty()) {
    	pInci = lst[0]->clone();
    }
    else {
    	LOG_WARNING("VCal to Incidence Conversion Failed ");
    }
    return pInci;
}

KCal::Incidence* CalendarBackend::getIncidenceFromIcal( const QString& aIString )
{
	FUNCTION_CALL_TRACE;

	KCal::Incidence* pInci = NULL;

    KCal::CalendarLocal tempCalendar(KDateTime::Spec::LocalZone());
    KCal::ICalFormat icf;
    icf.fromString(&tempCalendar, aIString);
    KCal::Incidence::List lst = tempCalendar.rawIncidences();

    if(!lst.isEmpty()) {
    	pInci = lst[0]->clone();
    } else {
    	LOG_WARNING("ICal to Incidence Conversion Failed ");
    }

    return pInci;
}

bool CalendarBackend::addIncidence( KCal::Incidence* aInci, bool commitNow )
{
    FUNCTION_CALL_TRACE;

    if( !iCalendar || !iStorage ) {
        return false;
    }

    if( !iCalendar->addIncidence( aInci ) ) {
        LOG_WARNING( "Could not add incidence to calendar");
        return false;
    }

    // if you add an incidence, that incidence will be owned by calendar
    // so no need to delete it. Committing for each modification, can cause performance
    // problems.

    iCalendar->setNotebook( aInci, iNotebookStr );
    if( commitNow )  {
        if( !iStorage->save() )
        {
            LOG_WARNING( "Could not commit changes to calendar");
            return false;
        }
        LOG_DEBUG( "Single incidence committed");
    }

    LOG_DEBUG("Added an item with UID ***\n" << aInci->uid());

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

bool CalendarBackend::modifyIncidence( KCal::Incidence* aInci, const QString& aUID, bool commitNow )
{
	FUNCTION_CALL_TRACE;

    if( !iCalendar || !iStorage ) {
        return false;
    }

    KCal::Incidence* origInci = iCalendar->incidence( aUID );

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

bool CalendarBackend::deleteIncidence( const QString& aUID )
{
	FUNCTION_CALL_TRACE;

    if( !iCalendar || !iStorage ) {
        return false;
    }

    KCal::Incidence* incidence = iCalendar->incidence( aUID );

    if( !incidence ) {
        LOG_WARNING( "Could not find incidence to delete with UID" << aUID );
        return false;
    }

    if( !iCalendar->deleteIncidence( incidence) )
    {
        LOG_WARNING( "Could not delete incidence with UID" << aUID );
        return false;
    }

    if( !iStorage->save() ) {
        LOG_WARNING( "Could not commit changes to calendar");
        return false;
    }

    return true;
}

bool CalendarBackend::modifyIncidence( KCal::Incidence* aIncidence, KCal::Incidence* aIncidenceData )
{
	FUNCTION_CALL_TRACE;

	Q_ASSERT( aIncidence );
	Q_ASSERT( aIncidenceData );

	// Save critical data from original item
	aIncidenceData->setUid( aIncidence->uid() );
	aIncidenceData->setCreated( aIncidence->created() );

	// This was needed previously for modify to work, but the function is not available anymore.
	// @todo: verify if modify works with latest libmaemokcal
	//aIncidenceData->setRowId( aIncidence->rowId() );

	if( aIncidence->type() != aIncidenceData->type() ) {
	    LOG_WARNING( "Expected incidence type" << aIncidence->type() <<", got" << aIncidenceData->type() );
	    return false;
	}

    if( aIncidence->type() == INCIDENCE_TYPE_EVENT )
    {
        KCal::Event* inc = static_cast<KCal::Event*>( aIncidence );
        KCal::Event* data = static_cast<KCal::Event*>( aIncidenceData );
        *inc = *data;
    }
    else if( aIncidence->type() == INCIDENCE_TYPE_TODO )
    {
        KCal::Todo* inc = static_cast<KCal::Todo*>( aIncidence );
        KCal::Todo* data = static_cast<KCal::Todo*>( aIncidenceData );
        *inc = *data;
    }
    else {
        LOG_WARNING( "Unsupported incidence type:" << aIncidence->type() );
        return false;
    }

    iCalendar->setNotebook( aIncidence, iNotebookStr );

    return true;

}

void CalendarBackend::printCalendar()
{
	FUNCTION_CALL_TRACE;

    if( iCalendar ) {

        KCal::ICalFormat ical;
        QString str = ical.toString(iCalendar);
        LOG_DEBUG(str.toLatin1().data());

    }

}

void CalendarBackend::filterIncidences(KCal::Incidence::List& aList)
{
	FUNCTION_CALL_TRACE;
	QString event(INCIDENCE_TYPE_EVENT);
	QString todo(INCIDENCE_TYPE_TODO);
    foreach(KCal::Incidence* incidence, aList) {
    	if ((incidence->type() != event) && (incidence->type() != todo)) {
    		LOG_DEBUG("Removing incidence type" << incidence->type());
            aList.removeRef( incidence );
            // Removed incidences are deleted by the list that is in
            // auto delete mode.
        }
    }
}
