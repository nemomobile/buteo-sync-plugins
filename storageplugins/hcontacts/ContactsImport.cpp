//
// This implementation is taken from seasideimport.cpp in https://github.com/nemomobile/libcontacts
//

/*
 * Copyright (C) 2015 Jolla Mobile <matthew.vogt@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "ContactsImport.h"
#include "ContactPropertyHandler.h"

#define USING_QTPIM QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

#include <QContactDetailFilter>
#include <QContactFetchHint>
#include <QContactManager>
#include <QContactSortOrder>
#include <QContactSyncTarget>

#include <QContactAddress>
#include <QContactAnniversary>
#include <QContactAvatar>
#include <QContactBirthday>
#include <QContactEmailAddress>
#include <QContactFamily>
#include <QContactGeoLocation>
#include <QContactGuid>
#include <QContactHobby>
#include <QContactName>
#include <QContactNickname>
#include <QContactNote>
#include <QContactOnlineAccount>
#include <QContactOrganization>
#include <QContactPhoneNumber>
#include <QContactRingtone>
#include <QContactTag>
#include <QContactUrl>

#ifdef USING_QTPIM
#include <QContactIdFilter>
#include <QContactExtendedDetail>
#else
#include <QContactLocalIdFilter>
#endif

#include <QVersitContactExporter>
#include <QVersitContactImporter>
#include <QVersitReader>
#include <QVersitWriter>

#include <QHash>
#include <QString>

namespace {

QContactFetchHint basicFetchHint()
{
    QContactFetchHint fetchHint;

    fetchHint.setOptimizationHints(QContactFetchHint::NoRelationships |
                                   QContactFetchHint::NoActionPreferences |
                                   QContactFetchHint::NoBinaryBlobs);

    return fetchHint;
}

bool nameIsEmpty(const QContactName &name)
{
    if (name.isEmpty())
        return true;

    return (name.prefix().isEmpty() &&
            name.firstName().isEmpty() &&
            name.middleName().isEmpty() &&
            name.lastName().isEmpty() &&
            name.suffix().isEmpty());
}

QString contactNameString(const QContact &contact)
{
    QStringList details;
    QContactName name(contact.detail<QContactName>());
    if (nameIsEmpty(name))
        return QString();

    details.append(name.prefix());
    details.append(name.firstName());
    details.append(name.middleName());
    details.append(name.lastName());
    details.append(name.suffix());
    return details.join(QChar::fromLatin1('|'));
}


template<typename T, typename F>
QVariant detailValue(const T &detail, F field)
{
#ifdef USING_QTPIM
    return detail.value(field);
#else
    return detail.variantValue(field);
#endif
}

#ifdef USING_QTPIM
typedef QMap<int, QVariant> DetailMap;
#else
typedef QVariantMap DetailMap;
#endif

DetailMap detailValues(const QContactDetail &detail)
{
#ifdef USING_QTPIM
    DetailMap rv(detail.values());
#else
    DetailMap rv(detail.variantValues());
#endif
    return rv;
}

static bool variantEqual(const QVariant &lhs, const QVariant &rhs)
{
#ifdef USING_QTPIM
    // Work around incorrect result from QVariant::operator== when variants contain QList<int>
    static const int QListIntType = QMetaType::type("QList<int>");

    const int lhsType = lhs.userType();
    if (lhsType != rhs.userType()) {
        return false;
    }

    if (lhsType == QListIntType) {
        return (lhs.value<QList<int> >() == rhs.value<QList<int> >());
    }
#endif
    return (lhs == rhs);
}

static bool detailValuesSuperset(const QContactDetail &lhs, const QContactDetail &rhs)
{
    // True if all values in rhs are present in lhs
    const DetailMap lhsValues(detailValues(lhs));
    const DetailMap rhsValues(detailValues(rhs));

    if (lhsValues.count() < rhsValues.count()) {
        return false;
    }

    foreach (const DetailMap::key_type &key, rhsValues.keys()) {
        if (!variantEqual(lhsValues[key], rhsValues[key])) {
            return false;
        }
    }

    return true;
}

static void fixupDetail(QContactDetail &)
{
}

#ifdef USING_QTPIM
// Fixup QContactUrl because importer produces incorrectly typed URL field
static void fixupDetail(QContactUrl &url)
{
    QVariant urlField = url.value(QContactUrl::FieldUrl);
    if (!urlField.isNull()) {
        QString urlString = urlField.toString();
        if (!urlString.isEmpty()) {
            url.setValue(QContactUrl::FieldUrl, QUrl(urlString));
        } else {
            url.setValue(QContactUrl::FieldUrl, QVariant());
        }
    }
}

// Fixup QContactOrganization because importer produces invalid department
static void fixupDetail(QContactOrganization &org)
{
    QVariant deptField = org.value(QContactOrganization::FieldDepartment);
    if (!deptField.isNull()) {
        QStringList deptList = deptField.toStringList();

        // Remove any empty elements from the list
        QStringList::iterator it = deptList.begin();
        while (it != deptList.end()) {
            if ((*it).isEmpty()) {
                it = deptList.erase(it);
            } else {
                ++it;
            }
        }

        if (!deptList.isEmpty()) {
            org.setValue(QContactOrganization::FieldDepartment, deptList);
        } else {
            org.setValue(QContactOrganization::FieldDepartment, QVariant());
        }
    }
}
#endif

template<typename T>
bool updateExistingDetails(QContact *updateContact, const QContact &importedContact, bool singular = false)
{
    bool rv = false;

    QList<T> existingDetails(updateContact->details<T>());
    if (singular && !existingDetails.isEmpty())
        return rv;

    foreach (T detail, importedContact.details<T>()) {
        // Make any corrections to the input
        fixupDetail(detail);

        // See if the contact already has a detail which is a superset of this one
        bool found = false;
        foreach (const T &existing, existingDetails) {
            if (detailValuesSuperset(existing, detail)) {
                found = true;
                break;
            }
        }
        if (!found) {
            updateContact->saveDetail(&detail);
            rv = true;
        }
    }

    return rv;
}

bool mergeIntoExistingContact(QContact *updateContact, const QContact &importedContact)
{
    bool rv = false;

    // Update the existing contact with any details in the new import
    rv |= updateExistingDetails<QContactAddress>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactAnniversary>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactAvatar>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactBirthday>(updateContact, importedContact, true);
    rv |= updateExistingDetails<QContactEmailAddress>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactFamily>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactGeoLocation>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactGuid>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactHobby>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactNickname>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactNote>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactOnlineAccount>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactOrganization>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactPhoneNumber>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactRingtone>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactTag>(updateContact, importedContact);
    rv |= updateExistingDetails<QContactUrl>(updateContact, importedContact);
#ifdef USING_QTPIM
    rv |= updateExistingDetails<QContactExtendedDetail>(updateContact, importedContact);
#endif

    return rv;
}

bool updateExistingContact(QContact *updateContact, const QContact &contact)
{
    // Replace the imported contact with the existing version
    QContact importedContact(*updateContact);
    *updateContact = contact;

    return mergeIntoExistingContact(updateContact, importedContact);
}

QString generateDisplayLabelFromNonNameDetails(const QContact &contact)
{
    foreach (const QContactNickname& nickname, contact.details<QContactNickname>()) {
        if (!nickname.nickname().isEmpty()) {
            return nickname.nickname();
        }
    }

    // If this contact has organization details but no name, it probably represents that organization directly
    QContactOrganization company = contact.detail<QContactOrganization>();
    if (!company.name().isEmpty()) {
        return company.name();
    }

    // If none of the detail fields provides a label, fallback to the backend's label string, in
    // preference to using any of the addressing details directly
    const QString displayLabel = contact.detail<QContactDisplayLabel>().label();
    if (!displayLabel.isEmpty()) {
        return displayLabel;
    }

    foreach (const QContactOnlineAccount& account, contact.details<QContactOnlineAccount>()) {
        if (!account.accountUri().isEmpty()) {
            return account.accountUri();
        }
    }

    foreach (const QContactEmailAddress& email, contact.details<QContactEmailAddress>()) {
        if (!email.emailAddress().isEmpty()) {
            return email.emailAddress();
        }
    }

    foreach (const QContactPhoneNumber& phone, contact.details<QContactPhoneNumber>()) {
        if (!phone.number().isEmpty())
            return phone.number();
    }

    return QString();
}

void setNickname(QContact &contact, const QString &text)
{
    foreach (const QContactNickname &nick, contact.details<QContactNickname>()) {
        if (nick.nickname() == text) {
            return;
        }
    }

    QContactNickname nick;
    nick.setNickname(text);
    contact.saveDetail(&nick);
}

bool allCharactersMatchScript(const QString &s, QChar::Script script)
{
    for (int i=0; i<s.length(); i++) {
        if (s[i].script() != script) {
            return false;
        }
    }
    return true;
}

bool applyNameFixes(QContactName *nameDetail)
{
    // Chinese names shouldn't have a middle name, so if it is present in a Han-script-only
    // name, it is probably wrong and it should be prepended to the first name instead.
    QString middleName = nameDetail->middleName();
    if (middleName.isEmpty()) {
        return false;
    }
    QString firstName = nameDetail->firstName();
    QString lastName = nameDetail->lastName();
    if (!allCharactersMatchScript(middleName, QChar::Script_Han)
            || (!firstName.isEmpty() && !allCharactersMatchScript(firstName, QChar::Script_Han))
            || (!lastName.isEmpty() && !allCharactersMatchScript(lastName, QChar::Script_Han))) {
        return false;
    }
    nameDetail->setFirstName(middleName + firstName);
    nameDetail->setMiddleName(QString());
    return true;
}

}

QList<QContact> ContactsImport::buildImportContacts(QContactManager *mgr,
                                                   const QList<QContact> &newContacts,
                                                   const QContactFilter &contactFilter,
                                                   int *newCount,
                                                   int *updatedCount)
{
    if (newCount)
        *newCount = 0;
    if (updatedCount)
        *updatedCount = 0;

    QList<QContact> importedContacts = newContacts;

    QHash<QString, int> importGuids;
    QHash<QString, int> importNames;
    QHash<QString, int> importLabels;

    QSet<QContactDetail::DetailType> unimportableDetailTypes;
    unimportableDetailTypes.insert(QContactDetail::TypeGlobalPresence);
    unimportableDetailTypes.insert(QContactDetail::TypeVersion);

    // Merge any duplicates in the import list
    QList<QContact>::iterator it = importedContacts.begin();
    while (it != importedContacts.end()) {
        QContact &contact(*it);

        // Fix up name (field ordering) if required
        QContactName nameDetail = contact.detail<QContactName>();
        if (applyNameFixes(&nameDetail)) {
            contact.saveDetail(&nameDetail);
        }

        // Remove any details that our backend can't store
        foreach (QContactDetail detail, contact.details()) {
            if (detail.type() == QContactSyncTarget::Type) {
                qDebug() << "  Removing unimportable syncTarget:" << detail;
                contact.removeDetail(&detail);
            } else if (unimportableDetailTypes.contains(detail.type())) {
                qDebug() << "  Removing unimportable detail:" << detail;
                contact.removeDetail(&detail);
            }
        }

        const QString guid = contact.detail<QContactGuid>().guid();
        const QString name = contactNameString(contact);
        const bool emptyName = name.isEmpty();

        QString label;
        if (emptyName) {
            QContactName nameDetail = contact.detail<QContactName>();
            contact.removeDetail(&nameDetail);

            label = contact.detail<QContactDisplayLabel>().label();
            if (label.isEmpty()) {
                label = generateDisplayLabelFromNonNameDetails(contact);
            }
        }

        int previousIndex = -1;
        QHash<QString, int>::const_iterator git = importGuids.find(guid);
        if (git != importGuids.end()) {
            previousIndex = git.value();

            if (!emptyName) {
                // If we have a GUID match, but names differ, ignore the match
                const QContact &previous(importedContacts[previousIndex]);
                const QString previousName = contactNameString(previous);
                if (!previousName.isEmpty() && (previousName != name)) {
                    previousIndex = -1;

                    // Remove the conflicting GUID from this contact
                    QContactGuid guidDetail = contact.detail<QContactGuid>();
                    contact.removeDetail(&guidDetail);
                }
            }
        }
        if (previousIndex == -1) {
            if (!emptyName) {
                QHash<QString, int>::const_iterator nit = importNames.find(name);
                if (nit != importNames.end()) {
                    previousIndex = nit.value();
                }
            } else if (!label.isEmpty()) {
                // Only if name is empty, use displayLabel
                QHash<QString, int>::const_iterator lit = importLabels.find(label);
                if (lit != importLabels.end()) {
                    previousIndex = lit.value();
                }
            }
        }

        if (previousIndex != -1) {
            // Combine these duplicate contacts
            QContact &previous(importedContacts[previousIndex]);
            mergeIntoExistingContact(&previous, contact);

            it = importedContacts.erase(it);
        } else {
            const int index = it - importedContacts.begin();
            if (!guid.isEmpty()) {
                importGuids.insert(guid, index);
            }
            if (!emptyName) {
                importNames.insert(name, index);
            } else if (!label.isEmpty()) {
                importLabels.insert(label, index);

                if (contact.details<QContactNickname>().isEmpty()) {
                    // Modify this contact to have the label as a nickname
                    setNickname(contact, label);
                }
            }

            ++it;
        }
    }

    // Find all names and GUIDs for local contacts that might match these contacts
    QContactFetchHint fetchHint(basicFetchHint());
#ifdef USING_QTPIM
    fetchHint.setDetailTypesHint(QList<QContactDetail::DetailType>() << QContactName::Type << QContactNickname::Type << QContactGuid::Type);
#else
    fetchHint.setDetailDefinitionsHint(QStringList() << QContactName::DefinitionName << QContactNickname::DefinitionName << QContactGuid::DefinitionName);
#endif

    QHash<QString, QContactId> existingGuids;
    QHash<QString, QContactId> existingNames;
    QMap<QContactId, QString> existingContactNames;
    QHash<QString, QContactId> existingNicknames;

    foreach (const QContact &contact, mgr->contacts(contactFilter, QList<QContactSortOrder>(), fetchHint)) {
        const QString guid = contact.detail<QContactGuid>().guid();
        const QString name = contactNameString(contact);

        if (!guid.isEmpty()) {
            existingGuids.insert(guid, contact.id());
        }
        if (!name.isEmpty()) {
            existingNames.insert(name, contact.id());
            existingContactNames.insert(contact.id(), name);
        }
        foreach (const QContactNickname &nick, contact.details<QContactNickname>()) {
            existingNicknames.insert(nick.nickname(), contact.id());
        }
    }

    // Find any imported contacts that match contacts we already have
    QMap<QContactId, int> existingIds;

    it = importedContacts.begin();
    while (it != importedContacts.end()) {
        const QString guid = (*it).detail<QContactGuid>().guid();
        const QString name = contactNameString(*it);
        const bool emptyName = name.isEmpty();

        QContactId existingId;

        QHash<QString, QContactId>::const_iterator git = existingGuids.find(guid);
        if (git != existingGuids.end()) {
            existingId = *git;

            if (!emptyName) {
                // If we have a GUID match, but names differ, ignore the match
                QMap<QContactId, QString>::iterator nit = existingContactNames.find(existingId);
                if (nit != existingContactNames.end()) {
                    const QString &existingName(*nit);
                    if (!existingName.isEmpty() && (existingName != name)) {
                        existingId = QContactId();

                        // Remove the conflicting GUID from this contact
                        QContactGuid guidDetail = (*it).detail<QContactGuid>();
                        (*it).removeDetail(&guidDetail);
                    }
                }
            }
        }
        if (existingId.isNull()) {
            if (!emptyName) {
                QHash<QString, QContactId>::const_iterator nit = existingNames.find(name);
                if (nit != existingNames.end()) {
                    existingId = *nit;
                }
            } else {
                foreach (const QContactNickname nick, (*it).details<QContactNickname>()) {
                    const QString nickname(nick.nickname());
                    if (!nickname.isEmpty()) {
                        QHash<QString, QContactId>::const_iterator nit = existingNicknames.find(nickname);
                        if (nit != existingNicknames.end()) {
                            existingId = *nit;
                            break;
                        }
                    }
                }
            }
        }

        if (!existingId.isNull()) {
            QMap<QContactId, int>::iterator eit = existingIds.find(existingId);
            if (eit == existingIds.end()) {
                existingIds.insert(existingId, (it - importedContacts.begin()));

                ++it;
            } else {
                // Combine these contacts with matching names
                QContact &previous(importedContacts[*eit]);
                mergeIntoExistingContact(&previous, *it);

                it = importedContacts.erase(it);
            }
        } else {
            ++it;
        }
    }

    int existingCount(existingIds.count());
    if (existingCount > 0) {
        // Retrieve all the contacts that we have matches for
#ifdef USING_QTPIM
        QContactIdFilter idFilter;
        idFilter.setIds(existingIds.keys());
#else
        QContactLocalIdFilter idFilter;
        QList<QContactLocalId> localIds;
        foreach (const QContactId &id, existingIds.keys()) {
            localids.append(id.toLocal());
        }
#endif

        QSet<QContactId> modifiedContacts;
        QSet<QContactId> unmodifiedContacts;

        foreach (const QContact &contact, mgr->contacts(idFilter & contactFilter, QList<QContactSortOrder>(), basicFetchHint())) {
            QMap<QContactId, int>::const_iterator it = existingIds.find(contact.id());
            if (it != existingIds.end()) {
                // Update the existing version of the contact with any new details
                QContact &importContact(importedContacts[*it]);
                bool modified = updateExistingContact(&importContact, contact);
                if (modified) {
                    modifiedContacts.insert(importContact.id());
                } else {
                    unmodifiedContacts.insert(importContact.id());
                }
            } else {
                qWarning() << "unable to update existing contact:" << contact.id();
            }
        }

        if (!unmodifiedContacts.isEmpty()) {
            QList<QContact>::iterator it = importedContacts.begin();
            while (it != importedContacts.end()) {
                const QContact &importContact(*it);
                const QContactId contactId(importContact.id());

                if (unmodifiedContacts.contains(contactId) && !modifiedContacts.contains(contactId)) {
                    // This contact was not modified by import - don't update it
                    it = importedContacts.erase(it);
                    --existingCount;
                } else {
                    ++it;
                }
            }
        }
    }

    if (updatedCount)
        *updatedCount = existingCount;
    if (newCount)
        *newCount = importedContacts.count() - existingCount;

    return importedContacts;
}

