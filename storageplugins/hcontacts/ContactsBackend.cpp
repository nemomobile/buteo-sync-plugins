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
#include "ContactsImport.h"
#include <LogMacros.h>
#include <QVersitContactExporter>
#include <QVersitContactImporter>
#include <QVersitReader>
#include <QVersitWriter>
#include <QContactTimestamp>
#include <QContactSyncTarget>
#include <QContactDetailFilter>
#include <QContactUnionFilter>
#include <QBuffer>
#include <QSet>

#include <qtcontacts-extensions.h>
#include <qtcontacts-extensions_impl.h>
#include <QContactOriginMetadata>
#include <qcontactoriginmetadata_impl.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QContactIdFilter>
#define QContactLocalIdFilter QContactIdFilter
#else
#include <QContactLocalIdFilter>
#endif

#include "ContactDetailHandler.h"

ContactsBackend::ContactsBackend(QVersitDocument::VersitType aVCardVer, const QString &syncTarget, const QString &originId) :
iMgr(NULL) ,iVCardVer(aVCardVer) //CID 26531
    , iSyncTarget(syncTarget)
    , iOriginId(originId)
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

        iMgr = new QContactManager(QLatin1String("org.nemomobile.contacts.sqlite"));

        if(iMgr != NULL)
            return true;
        else
            return false;
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
    } else {
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

    QList<QContactId> oldContacts;
    if (!iOriginId.isEmpty()) {
        QContactDetailFilter originIdFilter = QContactOriginMetadata::matchId(iOriginId);
        oldContacts = iMgr->contactIds(originIdFilter & getSyncTargetFilter());
    }

    int newCount = 0;
    int updatedCount = 0;
    QList<QContact> newContacts = convertVCardListToQContactList(aContactDataList);

    // Import the vcard data into existing contacts. We want to do a careful merge instead of
    // overwriting existing contacts to avoid losing data about user-applied contact links.
    QContactDetailFilter originIdFilter = QContactOriginMetadata::matchId(iOriginId);
    QList<QContact> contactList = ContactsImport::buildImportContacts(iMgr, newContacts, originIdFilter & getSyncTargetFilter(), &newCount, &updatedCount);
    prepareContactSave(&contactList);

    LOG_DEBUG("New contacts:" << newCount << "Updated contacts:" << updatedCount);

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QContactLocalId contactId = contactList.at(i).id();
        status.id = contactId.toString ();
#else
        QContactLocalId contactId = contactList.at(i).id().localId();
        status.id = (int)contactId;
#endif
        if (!errorMap.contains(i))
        {
            status.errorCode = QContactManager::NoError;
        }
        else
        {
            LOG_WARNING("Contact with id " << contactId << " and index " << i <<" is in error");
            QContactManager::Error errorCode = errorMap.value(i);
            status.errorCode = errorCode;
        }
        aStatusMap.insert(i, status);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        getContact(QContactId::fromString (aID), oldContactData);
#else
        getContact(aID.toUInt(), oldContactData);
#endif
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

    Q_ASSERT (iMgr);
    ContactsStatus status;

    QMap<int,QContactManager::Error> errors;
    QMap<int,ContactsStatus> statusMap;

    if (aVCardDataList.size() == aContactIdList.size()) {

        QList<QContact> qContactList = convertVCardListToQContactList(aVCardDataList);

        for (int i = 0; i < qContactList.size(); i++) {
            LOG_DEBUG("Id of the contact to be replaced" << aContactIdList.at(i));
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            QContactLocalId uniqueContactItemID = QContactId::fromString (aContactIdList.at(i));
#else
            QContactLocalId managerLocalIdOfItemBeingModified = aContactIdList.at(i).toUInt();
            QContactId uniqueContactItemID;
            uniqueContactItemID.setLocalId(managerLocalIdOfItemBeingModified);
#endif

            qContactList[i].setId(uniqueContactItemID);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            LOG_DEBUG("Replacing item's ID " << qContactList.at(i));
#else
            LOG_DEBUG("Replacing item's ID " << qContactList.at(i).localId());
#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            QContactLocalId contactId = qContactList.at(i).id();
            status.id = contactId.toString ();
#else
            QContactLocalId contactId = qContactList.at(i).id().localId();
            status.id = (int)contactId;
#endif
            if( !errors.contains(i) )
            {
                LOG_DEBUG("No error for contact with id " << contactId << " and index " << i);
                status.errorCode = QContactManager::NoError;
            }
            else
            {
                LOG_DEBUG("contact with id " << contactId << " and index " << i <<" is in error");
                QContactManager::Error errorCode = errors.value(i);
                status.errorCode = errorCode;
            }
            statusMap.insert(i, status);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            qContactIdList.append(QContactLocalId::fromString (id));
#else
            qContactIdList.append(QContactLocalId(id.toUInt()));
#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            status.id = contactId.toString ();
#else
            status.id = (int)contactId;
#endif
            if( !errors.contains(i) )
            {
                LOG_DEBUG("No error for contact with id " << contactId << " and index " << i);
                status.errorCode = QContactManager::NoError;
            }
            else
            {
                LOG_DEBUG("contact with id " << contactId << " and index " << i <<" is in error");
                QContactManager::Error errorCode = errors.value(i);
                status.errorCode = errorCode;
            }
            statusMap.insert(i, status);
        }
    }

        return statusMap;
}

void ContactsBackend::prepareContactSave(QList<QContact> *contactList)
{
    if (!iSyncTarget.isEmpty() || !iOriginId.isEmpty()) {
        for (int i=0; i<contactList->count(); i++) {
            QContact *contact = &((*contactList)[i]);
            if (!iSyncTarget.isEmpty()) {
                QContactSyncTarget syncTarget = contact->detail<QContactSyncTarget>();
                syncTarget.setSyncTarget(iSyncTarget);
                contact->saveDetail(&syncTarget);
            }
            if (!iOriginId.isEmpty()) {
                QContactOriginMetadata originMetaData = contact->detail<QContactOriginMetadata>();
                originMetaData.setId(iOriginId);
                contact->saveDetail(&originMetaData);
            }
        }
    }
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
        prepareContactSave(&contactList);
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

        ContactDetailHandler handler;
        contactExporter.setDetailHandler(&handler);

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

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
QMap<QString, QString> ContactsBackend::convertQContactListToVCardList(
    const QList<QContact> & aContactList)
#else
QMap<QContactLocalId, QString> ContactsBackend::convertQContactListToVCardList(
    const QList<QContact> & aContactList)
#endif
{
        FUNCTION_CALL_TRACE;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QMap<QString, QString> idDataMap;
#else
        QMap<QContactLocalId, QString> idDataMap;
#endif

        foreach (QContact contact, aContactList) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                idDataMap[contact.id ().toString ()] = convertQContactToVCard(contact);
#else
                idDataMap[contact.localId()] = convertQContactToVCard(contact);
#endif
        }

        return idDataMap;
}

void ContactsBackend::getSpecifiedContactIds(const QContactChangeLogFilter::EventType aEventType,
                const QDateTime& aTimeStamp, QList<QContactLocalId>& aIdList)
{
        FUNCTION_CALL_TRACE;

        QContactChangeLogFilter filter(aEventType);
        filter.setSince(aTimeStamp);

    aIdList = iMgr->contactIds(filter & getSyncTargetFilter());

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // Fetch the ids from aIdList
    QList<QString> strIdList;
    foreach (const QContactId& id, aIdList) {
        strIdList << id.toString ();
    }
#endif

    // Filter out ids for items that were added after the specified time.
    if (aEventType != QContactChangeLogFilter::EventAdded)
    {
        filter.setEventType(QContactChangeLogFilter::EventAdded);
        QList<QContactLocalId> addedList = iMgr->contactIds(filter & getSyncTargetFilter());
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QList<QString> addedStrIdList;
        foreach (const QContactId& id, addedList) {
            addedStrIdList << id.toString ();
        }

        foreach (const QString addedId, addedStrIdList) {
            strIdList.removeAll (addedId);
        }
#endif
        foreach (const QContactLocalId &id, addedList)
        {
            aIdList.removeAll(id);
        }
    }

        // This is a defensive procedure to prevent duplicate items being sent.
        // QSet does not allow duplicates, thus transforming QList to QSet and back
        // again will remove any duplicate items in the original QList.
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    int originalIdCount = strIdList.size ();
    QSet<QString> idSet = strIdList.toSet ();
    int idCountAfterDupRemoval = idSet.size ();
    strIdList = idSet.toList ();

    LOG_DEBUG("Item IDs found (returned / incl. duplicates): " << idCountAfterDupRemoval << "/" << originalIdCount);

    if (originalIdCount != idCountAfterDupRemoval) {
        LOG_WARNING("Contacts backend returned duplicate items for requested list");
        LOG_WARNING("Duplicate item IDs have been removed");
    } // no else

    // Convert strIdList to aIdList (QContactId)
    foreach (const QString &id, strIdList) {
        aIdList << QContactId::fromString (id);
    }
#else
        int originalIdCount = aIdList.size();
        QSet<QContactLocalId> idSet = aIdList.toSet();
        int idCountAfterDupRemoval = idSet.size();

        LOG_DEBUG("Item IDs found (returned / incl. duplicates): " << idCountAfterDupRemoval << "/" << originalIdCount);

        if (originalIdCount != idCountAfterDupRemoval) {
                LOG_WARNING("Contacts backend returned duplicate items for requested list");
                LOG_WARNING("Duplicate item IDs have been removed");
        } // no else

        aIdList = idSet.toList();
#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        contactTimestamps = contact.detail (QContactTimestamp::Type);
#else
        QString definitionName = contactTimestamps.definitionName();
        contactTimestamps = (QContactTimestamp)contact.detail(definitionName);
#endif
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
        aContacts = iMgr->contacts(contactFilter & getSyncTargetFilter());
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void ContactsBackend::getContactsForExport(const QList<QContactLocalId>&  aIdsList,
                                  QMap<QString,QString>& aDataMap)
#else
void ContactsBackend::getContactsForExport(const QList<QContactLocalId>&  aIdsList,
                                  QMap<QContactLocalId,QString>& aDataMap)
#endif
{
    FUNCTION_CALL_TRACE;

    // XXX only return the ids reported by aIdsList
    QList<QContact> returnedContacts = retrieveAllPartialAggregates();
    aDataMap = convertQContactListToVCardList(returnedContacts);

//<<<<<<< Updated upstream
//    QList<QContact> returnedContacts;

//    // As this is an overloaded convenience function, these two functions
//    // are utilized to get contacts from the backend and to convert them
//    // to vcard format.
//    getContacts(aIdsList, returnedContacts);
//=======
////    QList<QContact> returnedContacts;
////    getContacts(aIdsList, returnedContacts);

//    QContactLocalIdFilter contactFilter;
//    contactFilter.setIds(aIdsList);
//    QList<QContact> returnedContacts = retrieveAllPartialAggregates();
////    if (iMgr != NULL) {
////        aggregatedContacts = iMgr->contacts(contactFilter & getSyncTargetFilter());
////    }


//>>>>>>> Stashed changes
//    aDataMap = convertQContactListToVCardList(returnedContacts);
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


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QList<QContactDetail::DetailType> detailTypes;
    detailTypes << QContactTimestamp::Type;
#else
    QContactTimestamp contactTimestampDef;
    QString definitionName = contactTimestampDef.definitionName();

    QStringList definitionNames;
    definitionNames.append( definitionName );
#endif

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

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    contactHint.setDetailTypesHint (detailTypes);
#else
    contactHint.setDetailDefinitionsHint( definitionNames );
#endif

    QDateTime currentTime = QDateTime::currentDateTime();

    contacts = iMgr->contacts( contactFilter, QList<QContactSortOrder>(), contactHint );

    if( contacts.count() == aContactIds.count() )
    {
        for( int i = 0; i < aContactIds.count(); ++i )
        {
            QDateTime creationTime = currentTime;

            for( int a = 0; a < contacts.count(); ++a )
            {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                if( contacts[a].id().toString () == aContactIds[i].toString () )
#else
                if( contacts[a].id().localId() == aContactIds[i] )
#endif
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

QContactFilter ContactsBackend::getSyncTargetFilter() const
{
    QContactDetailFilter detailFilterDefaultSyncTarget;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    detailFilterDefaultSyncTarget.setDetailType (QContactSyncTarget::Type,
                                                 QContactSyncTarget::FieldSyncTarget);
#else
    detailFilterDefaultSyncTarget.setDetailDefinitionName(QContactSyncTarget::DefinitionName,
                                         QContactSyncTarget::FieldSyncTarget);
#endif

    if (iSyncTarget.isEmpty()) {
        // contact data that is conceptually owned by
        // by the user (ie, "local" device contacts)
        detailFilterDefaultSyncTarget.setValue(QLatin1String("local"));
    } else {
        detailFilterDefaultSyncTarget.setValue(iSyncTarget);
    }
    return detailFilterDefaultSyncTarget;
}

QList<QContact> ContactsBackend::retrieveAllPartialAggregates()
{
    QMap<QContactId, QContact> localContacts;
    QMap<QContactId, QContact> waslocalContacts;
    QMap<QContactId, QContact> googleContacts;
    QMap<QContactId, QContact> aggregateContacts;

    // first, build up some data structures, containing all of the information we need.

    QContactDetailFilter aggregateFilter;
    aggregateFilter.setDetailType(QContactDetail::TypeSyncTarget, QContactSyncTarget::FieldSyncTarget);
    aggregateFilter.setValue(QLatin1String("aggregate"));
    QList<QContact> allAggregates = iMgr->contacts(aggregateFilter);
    for (int i = 0; i < allAggregates.size(); ++i)
        aggregateContacts.insert(allAggregates[i].id(), allAggregates[i]);

    QContactDetailFilter localFilter;
    localFilter.setDetailType(QContactDetail::TypeSyncTarget, QContactSyncTarget::FieldSyncTarget);
    localFilter.setValue(QLatin1String("local"));
    QList<QContact> allLocals = iMgr->contacts(localFilter);
    for (int i = 0; i < allLocals.size(); ++i)
        localContacts.insert(allLocals[i].id(), allLocals[i]);

    QContactDetailFilter waslocalFilter;
    waslocalFilter.setDetailType(QContactDetail::TypeSyncTarget, QContactSyncTarget::FieldSyncTarget);
    waslocalFilter.setValue(QLatin1String("was_local"));
    QList<QContact> allWasLocals = iMgr->contacts(waslocalFilter);
    for (int i = 0; i < allWasLocals.size(); ++i)
        waslocalContacts.insert(allWasLocals[i].id(), allWasLocals[i]);

    QContactDetailFilter googleFilter;
    googleFilter.setDetailType(QContactDetail::TypeSyncTarget, QContactSyncTarget::FieldSyncTarget);
    googleFilter.setValue(iSyncTarget);
    QList<QContact> allGoogles = iMgr->contacts(googleFilter);
    for (int i = 0; i < allGoogles.size(); ++i)
        googleContacts.insert(allGoogles[i].id(), allGoogles[i]);

    // now build up our list of partial aggregates, consisting of:
    // all partial aggregates for existing google contacts
    // all partial aggregates for existing local contacts which don't have an associated google contact
    QList<QContact> retn;
    QList<QContactId> localsRelatedToGoogles;

    // first, the existing ones:
    for (int i = 0; i < allGoogles.size(); ++i) {
        QContact partialAggregate = allGoogles.at(i);
//        if (!originIsThisAccount(partialAggregate, accountId)) {
//            // skip this one ??
//            // or automatically sync between accounts ??
//            // XXX TODO!
//        }

        QList<QContact> localConstituents = relatedLocals(partialAggregate, localContacts, waslocalContacts, aggregateContacts);
        for (int j = 0; j < localConstituents.size(); ++j) {
            QContact currLC = localConstituents.at(j);
            localsRelatedToGoogles.append(currLC.id());

            QList<QContactDetail> currentDetails = currLC.details();
            for (int k=0; k<currentDetails.count(); k++) {
                QContactDetail currDetail = currentDetails[k];
                // check that the partial aggregate doesn't
                // already have this detail (or one very similar)
                // If not, promote it!
                bool found = false;
                foreach (const QContactDetail &partialAggDetail, partialAggregate.details()) {
                    if (ContactsImport::detailValuesSuperset(partialAggDetail, currDetail)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    partialAggregate.saveDetail(&currDetail);
                }
            }
        }
        retn.append(partialAggregate);
    }

    // then, the locals for which we don't have a google contact yet.
    for (int i = 0; i < allLocals.size(); ++i) {
        QContact partialAggregate = allLocals.at(i);
        if (localsRelatedToGoogles.contains(partialAggregate.id())) {
            // skip this one - already has a google contact.
            continue;
        }

        // valid partial aggregate.
        QList<QContact> localConstituents = relatedLocals(partialAggregate, localContacts, waslocalContacts, aggregateContacts);
        for (int j = 0; j < localConstituents.size(); ++j) {
            QContact currLC = localConstituents.at(j);
            localsRelatedToGoogles.append(currLC.id());
//            foreach(QContactDetail &d, currLC.details()) {
//                // check that the partial aggregate doesn't
//                // already have this detail (or one very similar)
//                // If not, promote it!
//            }

            QList<QContactDetail> currentDetails = currLC.details();
            for (int k=0; k<currentDetails.count(); k++) {
                QContactDetail currDetail = currentDetails[k];
                // check that the partial aggregate doesn't
                // already have this detail (or one very similar)
                // If not, promote it!
                bool found = false;
                foreach (const QContactDetail &partialAggDetail, partialAggregate.details()) {
                    if (ContactsImport::detailValuesSuperset(partialAggDetail, currDetail)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    partialAggregate.saveDetail(&currDetail);
                }
            }
        }
        retn.append(partialAggregate);
    }

    // done.
    return retn;
}


QList<QContact> ContactsBackend::relatedLocals(const QContact &contact,
                              const QMap<QContactId, QContact> &locals,
                              const QMap<QContactId, QContact> &waslocals,
                              const QMap<QContactId, QContact> &aggregates)
{
    // Returns the local+was_local contacts which are related to the given google contact
    // Note that multiple google contacts can "share" the same local/was_local contacts,
    // if multiple google contacts get aggregated together.  But we still generate
    // a separate partial aggregate for each google contact, not each aggregate contact.
    QList<QContact> aggs = contact.relatedContacts(QLatin1String("Aggregates"));
    if (aggs.size() != 1) {
        qWarning() << "Don't have exactly one aggregate for the google contact!?";
        return QList<QContact>();
    }

    QContact aggregate = aggregates.value(aggs.first().id());
    QList<QContact> related = aggregate.relatedContacts(QLatin1String("Aggregates"));
    QList<QContact> retn;
    for (int i = 0; i < related.size(); ++i) {
        QContactId relatedId = related.at(i).id();
        if (contact.id() == relatedId) {
            // ignore this one, it's me.
            // needed because we call this function on local
            // as well as google synctarget contacts.
        } else if (locals.contains(relatedId)) {
            retn.append(locals.value(relatedId));
        } else if (waslocals.contains(relatedId)) {
            retn.append(waslocals.value(relatedId));
        }
    }

    return retn;
}
