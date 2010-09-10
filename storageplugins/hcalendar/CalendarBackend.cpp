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

    iCalendar = mKCal::ExtendedCalendar::Ptr( new mKCal::ExtendedCalendar( KDateTime::Spec::LocalZone( ) ));

    LOG_DEBUG("Creating Default Maemo Storage");
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

        iStorage.clear();

        LOG_TRACE("Storage deleted");

        iCalendar.clear();

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

    return iCalendar->incidence( aUID );
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

bool CalendarBackend::modifyIncidence( KCalCore::Incidence::Ptr aInci, const QString& aUID, bool commitNow )
{
	FUNCTION_CALL_TRACE;

    if( !iCalendar || !iStorage ) {
        return false;
    }

    KCalCore::Incidence::Ptr origInci = iCalendar->incidence( aUID );

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

    KCalCore::Incidence::Ptr incidence = iCalendar->incidence( aUID );

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

bool CalendarBackend::modifyIncidence( KCalCore::Incidence::Ptr& aIncidence, KCalCore::Incidence::Ptr& aIncidenceData )
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
	    LOG_WARNING( "Expected incidence type" << aIncidence->typeStr() <<", got" << aIncidenceData->typeStr() );
	    return false;
	}

    if( aIncidence->type() == KCalCore::Incidence::TypeEvent )
    {
        KCalCore::Event::Ptr inc = aIncidence.staticCast<KCalCore::Event>();
        KCalCore::Event::Ptr data = aIncidenceData.staticCast<KCalCore::Event>();
        inc = data;
    }
    else if( aIncidence->type() == KCalCore::Incidence::TypeTodo )
    {
        KCalCore::Todo::Ptr inc = aIncidence.staticCast<KCalCore::Todo>();
        KCalCore::Todo::Ptr data = aIncidenceData.staticCast<KCalCore::Todo>();
        inc = data;
    }
    else {
        LOG_WARNING( "Unsupported incidence type:" << aIncidence->typeStr() );
        return false;
    }

    iCalendar->setNotebook( aIncidence, iNotebookStr );

    return true;

}
