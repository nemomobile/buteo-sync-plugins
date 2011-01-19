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
#include "ContactsBackend.h"
#include <LogMacros.h>
#include <QVersitContactExporter>
#include <QVersitContactImporter>
#include <QVersitReader>
#include <QVersitWriter>
#include <QContactTimestamp>
#include <QContactLocalIdFilter>
#include <QContactSyncTarget>
#include <QContactDetailFilter>
#include <QBuffer>
#include <QSet>

ContactsBackend::ContactsBackend(QVersitDocument::VersitType aVCardVer) :
iMgr(NULL) ,iVCardVer(aVCardVer) //CID 26531
{
	FUNCTION_CALL_TRACE;
}

ContactsBackend::~ContactsBackend()
{
	FUNCTION_CALL_TRACE;
}

bool ContactsBackend::init()
{
	FUNCTION_CALL_TRACE;

	bool initStatus = false;
	QStringList availableManagers = QContactManager::availableManagers();

	if(availableManagers.contains("tracker")) {
		// hardcode to tracker
		LOG_DEBUG("connecting to storage tracker");
        iMgr = new QContactManager("tracker");

		if(iMgr != NULL){
			initStatus = true;

            if (!iMgr->hasFeature(QContactManager::ChangeLogs)) {
                LOG_CRITICAL( "Contacts manager does not support timestamps" );
            }
            else {
                LOG_DEBUG( "Contacts manager supports timestamps" );
            }
		}
		else {
			LOG_WARNING("Failed to connect to storage manager");
		}
	}

	return initStatus;
}

bool ContactsBackend::uninit()
{
	FUNCTION_CALL_TRACE;

	delete iMgr;
	iMgr = NULL;

	return true;
}



QList<QContactLocalId> ContactsBackend::getAllContactIds()
{
	FUNCTION_CALL_TRACE;
    QList<QContactLocalId> contactIDs;

    if (iMgr != NULL) {
        contactIDs = iMgr->contactIds(getSyncTargetFilter());
    }
    else {
        LOG_WARNING("Contacts backend not available");
    }
    
    return contactIDs;
}

QList<QContactLocalId> ContactsBackend::getAllNewContactIds(const QDateTime &aTimeStamp)
{
	FUNCTION_CALL_TRACE;

	LOG_DEBUG("Retrieve New Contacts Since " << aTimeStamp);

	QList<QContactLocalId> idList;
	const QContactChangeLogFilter::EventType eventType =
			QContactChangeLogFilter::EventAdded;

	getSpecifiedContactIds(eventType, aTimeStamp, idList);

	return idList;
}

QList<QContactLocalId> ContactsBackend::getAllModifiedContactIds(const QDateTime &aTimeStamp)
{

	FUNCTION_CALL_TRACE;

	LOG_DEBUG("Retrieve Modified Contacts Since " << aTimeStamp);

	QList<QContactLocalId> idList;
	const QContactChangeLogFilter::EventType eventType =
			QContactChangeLogFilter::EventChanged;

	getSpecifiedContactIds(eventType, aTimeStamp, idList);

	return idList;
}

QList<QContactLocalId> ContactsBackend::getAllDeletedContactIds(const QDateTime &aTimeStamp)
{
	FUNCTION_CALL_TRACE;

	LOG_DEBUG("Retrieve Deleted Contacts Since " << aTimeStamp);

	QList<QContactLocalId> idList;
	const QContactChangeLogFilter::EventType eventType =
			QContactChangeLogFilter::EventRemoved;

	getSpecifiedContactIds(eventType, aTimeStamp, idList);

	return idList;
}

bool ContactsBackend::addContacts( const QStringList& aContactDataList,
                                   QMap<int, ContactsStatus>& aStatusMap )
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( iMgr );

    QList<QContact> contactList = convertVCardListToQContactList(aContactDataList);
    ContactsStatus status;
    QMap<int, QContactManager::Error> errorMap;

    bool retVal = iMgr->saveContacts(&contactList, &errorMap);

    if (!retVal)
    {
        LOG_WARNING( "Errors reported while saving contacts:" << iMgr->error() );
    }

    // QContactManager will populate errorMap only for errors, but we use this as a status map,
    // so populate NoError if there's no error.
    // TODO QContactManager populates indices from the qContactList, but we populate keys, is this OK?
    for (int i = 0; i < contactList.size(); i++)
    {
        QContactLocalId contactId = contactList.at(i).id().localId();
        if (!errorMap.contains(i))
        {
            status.id = (int)contactId;
            status.errorCode = QContactManager::NoError;
            aStatusMap.insert(i, status);
        }
        else
        {
            LOG_WARNING("Contact with id " << contactId << " and index " << i <<" is in error");
            QContactManager::Error errorCode = errorMap.value(i);
            status.id = (int)contactId;
            status.errorCode = errorCode;
            aStatusMap.insert(i, status);
        }
    }

    return retVal;

}

QContactManager::Error ContactsBackend::modifyContact(const QString &aID, const QString &aContact)
{
	FUNCTION_CALL_TRACE;

	LOG_DEBUG("Modifying a Contact with ID" << aID);

    QContactManager::Error modificationStatus = QContactManager::UnspecifiedError;

    if (iMgr == NULL) {
        LOG_WARNING("Contacts backend not available");
    }
    else {
        QContact oldContactData;
        getContact(aID.toUInt(), oldContactData);
        QStringList contactStringList; 
        contactStringList.append(aContact);
        QContact newContactData = convertVCardListToQContactList(contactStringList).first();

        newContactData.setId(oldContactData.id());
        oldContactData = newContactData;

        bool modificationOk = iMgr->saveContact(&oldContactData);
        modificationStatus = iMgr->error();

        if(!modificationOk) {
            // either contact exists or something wrong with one of the detailed definitions
            LOG_WARNING("Contact Modification Failed");
        } // no else
    }

	return modificationStatus;
}

QMap<int,ContactsStatus> ContactsBackend::modifyContacts(
    const QStringList &aVCardDataList, const QStringList &aContactIdList)
{
	FUNCTION_CALL_TRACE;

    ContactsStatus status;
    
    QMap<int,QContactManager::Error> errors;
    QMap<int,ContactsStatus> statusMap;

    if (iMgr == NULL) {
        for (int i=0; i < aVCardDataList.size(); i++) {
            errors.insert(i, QContactManager::UnspecifiedError);
        }

        LOG_WARNING("Contacts backend not available");
    }
	else if (aVCardDataList.size() == aContactIdList.size()) {

		QList<QContact> qContactList = convertVCardListToQContactList(aVCardDataList);

		for (int i = 0; i < qContactList.size(); i++) {
			LOG_DEBUG("Id of the contact to be replaced" << aContactIdList.at(i));
			QContactLocalId managerLocalIdOfItemBeingModified = aContactIdList.at(i).toUInt();
            
			QContactId uniqueContactItemID;
			uniqueContactItemID.setLocalId(managerLocalIdOfItemBeingModified);
			qContactList[i].setId(uniqueContactItemID);
            
			LOG_DEBUG("Replacing item's ID " << qContactList.at(i).localId());
		}

		if(iMgr->saveContacts(&qContactList , &errors)) {
			LOG_DEBUG("Batch Modification of Contacts Succeeded");
		}
        else {
			LOG_DEBUG("Batch Modification of Contacts Failed");
		}

        // QContactManager will populate errorMap only for errors, but we use this as a status map,
	// so populate NoError if there's no error.
	// TODO QContactManager populates indices from the qContactList, but we populate keys, is this OK?
	for (int i = 0; i < qContactList.size(); i++) {
            QContactLocalId contactId = qContactList.at(i).id().localId();
            if( !errors.contains(i) )
            {
                LOG_DEBUG("No error for contact with id " << contactId << " and index " << i);
                status.id = (int)contactId;
                status.errorCode = QContactManager::NoError;
                statusMap.insert(i, status);
            }
            else 
            {
                LOG_DEBUG("contact with id " << contactId << " and index " << i <<" is in error");
                QContactManager::Error errorCode = errors.value(i);
                status.id = (int)contactId;
                status.errorCode = errorCode;
                statusMap.insert(i, status);
            }
        }
	}

	return statusMap;
}

QMap<int , ContactsStatus> ContactsBackend::deleteContacts(const QStringList &aContactIDList)
{
	FUNCTION_CALL_TRACE;

    ContactsStatus status;
    QMap<int , QContactManager::Error> errors;
    QMap<int , ContactsStatus> statusMap;
    
    if (iMgr == NULL) {
        for (int i=0; i < aContactIDList.size(); i++) {
            errors.insert(i, QContactManager::UnspecifiedError);
        }

        LOG_WARNING("Contacts backend not available");
    }
    else {
        QList<QContactLocalId> qContactIdList;
	foreach (QString id, aContactIDList ) {
	    qContactIdList.append(QContactLocalId(id.toUInt()));
        }

        if(iMgr->removeContacts(qContactIdList , &errors)) {
            LOG_DEBUG("Successfully Removed all contacts ");
        }
        else {
            LOG_WARNING("Failed Removing Contacts");
        }

        // QContactManager will populate errorMap only for errors, but we use this as a status map,
	// so populate NoError if there's no error.
	// TODO QContactManager populates indices from the qContactList, but we populate keys, is this OK?
	for (int i = 0; i < qContactIdList.size(); i++) {
            QContactLocalId contactId = qContactIdList.value(i);
            if( !errors.contains(i) )
            {
                LOG_DEBUG("No error for contact with id " << contactId << " and index " << i);
                status.id = (int)contactId;
                status.errorCode = QContactManager::NoError;
                statusMap.insert(i, status);
            }
            else 
            {
                LOG_DEBUG("contact with id " << contactId << " and index " << i <<" is in error");
                QContactManager::Error errorCode = errors.value(i);
                status.id = (int)contactId;
                status.errorCode = errorCode;
                statusMap.insert(i, status);
            }
        }
    }

	return statusMap;
}

QList<QContact> ContactsBackend::convertVCardListToQContactList(const QStringList &aVCardList)
{
    FUNCTION_CALL_TRACE;

    QByteArray byteArray;
    //QVersitReader needs LF/CRLF/CR between successive vcard's in the list,
    //CRLF didn't work though.
    const QString LF = "\n";

    foreach ( const QString& vcard, aVCardList )
    {
        byteArray.append(vcard.toUtf8());
        byteArray.append(LF.toUtf8());
    }

    QBuffer readBuf(&byteArray);
    readBuf.open(QIODevice::ReadOnly);
    readBuf.seek(0);

    QVersitReader versitReader;
    versitReader.setDevice (&readBuf);

    if (!versitReader.startReading())
    {
        LOG_WARNING ("Error while reading vcard");
    }

    if (!versitReader.waitForFinished() )
    {
        LOG_WARNING ("Error while finishing reading vcard");
    }

    QList<QVersitDocument> versitDocList = versitReader.results();
    readBuf.close();

    QVersitContactImporter contactImporter;
    QList<QContact> contactList;
    bool contactsImported = contactImporter.importDocuments(versitDocList);
    if (contactsImported)
    {
        contactList =  contactImporter.contacts();
        if (!contactList.isEmpty()) {
            foreach (QContact contact, contactList) {
                LOG_DEBUG("Converted item: " << contact.displayLabel());
            }
        } // no else

    }

    LOG_DEBUG( "Converted" << contactList.count() << "VCards" );

    return contactList;
}

QString ContactsBackend::convertQContactToVCard(const QContact &aContact)
{
	FUNCTION_CALL_TRACE;
	
	QList<QContact> contactsList;
	contactsList.append (aContact);
	
	QVersitContactExporter contactExporter;

	QString vCard;
	bool contactsExported = contactExporter.exportContacts(contactsList, iVCardVer);
	if (contactsExported){
		QList<QVersitDocument> versitDocumentList;
		versitDocumentList = contactExporter.documents();

		QBuffer writeBuf;
		writeBuf.open(QBuffer::ReadWrite);
	
		QVersitWriter writer;
		writer.setDevice(&writeBuf);

		if (!writer.startWriting(versitDocumentList)) {
			LOG_CRITICAL ("Error While writing -- " << writer.error() );
		}
	
		if (writer.waitForFinished()) {
			vCard = writeBuf.buffer();
		}
		
		writeBuf.close();
	}
	return vCard;
}

QMap<QContactLocalId, QString> ContactsBackend::convertQContactListToVCardList(
    const QList<QContact> & aContactList)
{
	FUNCTION_CALL_TRACE;
	QMap<QContactLocalId, QString> idDataMap;

	foreach (QContact contact, aContactList) {
		idDataMap[contact.localId()] = convertQContactToVCard(contact);
	}

	return idDataMap;
}

void ContactsBackend::getSpecifiedContactIds(const QContactChangeLogFilter::EventType aEventType,
		const QDateTime& aTimeStamp,
        QList<QContactLocalId>& aIdList)
{
	FUNCTION_CALL_TRACE;

	QContactChangeLogFilter filter(aEventType);
	filter.setSince(aTimeStamp);

    aIdList = iMgr->contactIds(filter & getSyncTargetFilter());

    // Filter out ids for items that were added after the specified time.
    if (aEventType != QContactChangeLogFilter::EventAdded)
    {
        filter.setEventType(QContactChangeLogFilter::EventAdded);
        QList<QContactLocalId> addedList = iMgr->contactIds(filter & getSyncTargetFilter());
        foreach (const QContactLocalId &id, addedList)
        {
            aIdList.removeAll(id);
        }
    }

	// This is a defensive procedure to prevent duplicate items being sent.
	// QSet does not allow duplicates, thus transforming QList to QSet and back
	// again will remove any duplicate items in the original QList.
	int originalIdCount = aIdList.size();
	QSet<QContactLocalId> idSet = aIdList.toSet();
	int idCountAfterDupRemoval = idSet.size();

	LOG_DEBUG("Item IDs found (returned / incl. duplicates): " << idCountAfterDupRemoval << "/" << originalIdCount);

	if (originalIdCount != idCountAfterDupRemoval) {
		LOG_WARNING("Contacts backend returned duplicate items for requested list");
		LOG_WARNING("Duplicate item IDs have been removed");
	} // no else

	aIdList = idSet.toList();
}

QDateTime ContactsBackend::lastModificationTime(const QContactLocalId &aContactId)
{
    FUNCTION_CALL_TRACE;

    QDateTime lastModificationTime = QDateTime::fromTime_t(0);

    if (iMgr == NULL) {
        LOG_WARNING("Contacts backend not available");
    }
    else {
        QContact contact;
        getContact(aContactId, contact);
        QContactTimestamp contactTimestamps;
        QString definitionName = contactTimestamps.definitionName();
        contactTimestamps = (QContactTimestamp)contact.detail(definitionName);
        lastModificationTime = contactTimestamps.lastModified();
    }

    return lastModificationTime;
}

/*!
    \fn ContactsBackend::getContact(QContactLocalId aContactId)
 */
void ContactsBackend::getContact(const QContactLocalId& aContactId, QContact& aContact)
{
    FUNCTION_CALL_TRACE;

    QList<QContactLocalId> contactId;
    contactId.append(aContactId);
    QList<QContact>        returnedContacts;

    getContacts(contactId, returnedContacts);

    if (!returnedContacts.isEmpty()) {
        aContact = returnedContacts.first();
    }

}

/*!
    \fn ContactsBackend::getContacts(QContactLocalId aContactId)
 */
void ContactsBackend::getContacts(const QList<QContactLocalId>& aContactIds,
                                  QList<QContact>& aContacts)
{
    FUNCTION_CALL_TRACE;
    
    QContactLocalIdFilter contactFilter;
    contactFilter.setIds(aContactIds);

    if (iMgr != NULL) {
        aContacts = iMgr->contacts(contactFilter);
    }
}

void ContactsBackend::getContacts(const QList<QContactLocalId>&  aIdsList,
                                  QMap<QContactLocalId,QString>& aDataMap)
{
    FUNCTION_CALL_TRACE;

    QList<QContact> returnedContacts;

    // As this is an overloaded convenience function, these two functions
    // are utilized to get contacts from the backend and to convert them
    // to vcard format.
    getContacts(aIdsList, returnedContacts);
    aDataMap = convertQContactListToVCardList(returnedContacts);

}

QDateTime ContactsBackend::getCreationTime( const QContact& aContact )
{
    FUNCTION_CALL_TRACE;

    QContactTimestamp contactTimestamp = aContact.detail<QContactTimestamp>();

    return contactTimestamp.created();
}

QList<QDateTime> ContactsBackend::getCreationTimes( const QList<QContactLocalId>& aContactIds )
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( iMgr );

    /* Retrieve QContacts from backend based on id's in aContactsIds. Since we're only interested
     * in timestamps, set up fetch hint accordingly to speed up the operation.
     */
    QList<QDateTime> creationTimes;
    QList<QContact> contacts;

    QContactLocalIdFilter contactFilter;
    contactFilter.setIds(aContactIds);

    QContactTimestamp contactTimestampDef;
    QString definitionName = contactTimestampDef.definitionName();

    /* Set up fetch hints so that not all details of QContacts be fetched:
     * 1) Fetch only QContactTimestamp details
     * 2) Do not try to resolve contact relationships (siblings etc)
     * 3) Do not include action preferences of contacts
     * 4) Do not fetch binary blogs (avatar pictures etc)
     */
    QContactFetchHint contactHint;
    contactHint.setOptimizationHints( QContactFetchHint::NoRelationships |
                                      QContactFetchHint::NoActionPreferences |
                                      QContactFetchHint::NoBinaryBlobs );

    QStringList definitionNames;
    definitionNames.append( definitionName );
    contactHint.setDetailDefinitionsHint( definitionNames );

    QDateTime currentTime = QDateTime::currentDateTime();

    contacts = iMgr->contacts( contactFilter, QList<QContactSortOrder>(), contactHint );

    if( contacts.count() == aContactIds.count() )
    {
        for( int i = 0; i < aContactIds.count(); ++i )
        {
            QDateTime creationTime = currentTime;

            for( int a = 0; a < contacts.count(); ++a )
            {
                if( contacts[a].id().localId() == aContactIds[i] )
                {
                    QContactTimestamp contactTimestamp = contacts[a].detail<QContactTimestamp>();
                    if( !contactTimestamp.created().isNull() &&
                        contactTimestamp.created().isValid() )
                    {
                        creationTime = contactTimestamp.created();
                    }
                    contacts.removeAt( a );
                    break;
                }
            }

            creationTimes.append( creationTime );
        }
    }
    else
    {
        LOG_WARNING( "Unable to fetch creation times" );
        for( int i = 0; i < aContactIds.count(); ++i )
        {
            creationTimes.append( currentTime );
        }
    }

    return creationTimes;
}

QContactFilter ContactsBackend::getSyncTargetFilter() const {
    // user enterred contacts, i.e. all other contacts that are not sourcing
    // from restricted backends or instant messaging service
    QContactDetailFilter detailFilterDefaultSyncTarget;
    detailFilterDefaultSyncTarget.setDetailDefinitionName(QContactSyncTarget::DefinitionName,
                                         QContactSyncTarget::FieldSyncTarget);
    detailFilterDefaultSyncTarget.setValue(QLatin1String("addressbook"));
    // "addressbook" - "magic" string from qtcontacts-tracker plugin

    // return the union
    return detailFilterDefaultSyncTarget;
}
