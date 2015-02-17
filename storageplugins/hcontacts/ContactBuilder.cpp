#include "ContactBuilder.h"

#include <QContactDetailFilter>
#include <QContactInvalidFilter>
#include <QContactSyncTarget>

#include <qtcontacts-extensions.h>
#include <qtcontacts-extensions_impl.h>
#include <QContactOriginMetadata>
#include <qcontactoriginmetadata_impl.h>

#include <seasidepropertyhandler.h>

ContactBuilder::ContactBuilder(QContactManager *mgr, const QString &syncTarget, const QString &originId, ContactBuilder::MatchFilterMode mode)
{
    QSet<QContactDetail::DetailType> ignoredDetailTypes = QSet<QContactDetail::DetailType>()
                                                          << QContactDetail::TypeGlobalPresence
                                                          << QContactDetail::TypePresence
                                                          << QContactDetail::TypeOnlineAccount
                                                          << QContactDetail::TypeVersion
                                                          << QContactDetail::TypeSyncTarget
                                                          << QContactDetail::TypeRingtone;

    d->manager = mgr;
    d->propertyHandler = new SeasidePropertyHandler(ignoredDetailTypes);
    d->unimportableDetailTypes = ignoredDetailTypes;
    d->importableSyncTargets = QStringList();

    d->extraData.insert("syncTarget", syncTarget);
    d->extraData.insert("originId", originId);
    d->extraData.insert("mode", static_cast<int>(mode));
}


ContactBuilder::~ContactBuilder()
{
    delete d->propertyHandler;
    d->propertyHandler = 0;
}


QContactFilter ContactBuilder::mergeSubsetFilter() const
{
    ContactBuilder::MatchFilterMode mode = static_cast<ContactBuilder::MatchFilterMode>(d->extraData.value("mode").toInt());
    if (mode == ContactBuilder::NoFilterRequiredMode) {
        // the caller already knows which id is for which contact
        // and we don't need to do any merging or matching.
        return QContactInvalidFilter();
    }

    // Import the vcard data into existing contacts. We want to do a careful merge instead of
    // overwriting existing contacts to avoid losing data about user-applied contact links.
    QString syncTarget = d->extraData.value("syncTarget").toString();
    QString originId = d->extraData.value("originId").toString();
    QContactDetailFilter syncTargetFilter;
    syncTargetFilter.setDetailType(QContactSyncTarget::Type, QContactSyncTarget::FieldSyncTarget);
    syncTargetFilter.setValue(syncTarget);
    QContactDetailFilter originIdFilter = QContactOriginMetadata::matchId(originId);

    // If the origin id is unknown, every contact is a new contact.  Otherwise, ensure we filter
    // based on the origin id as well as the synctarget, to minimise match surface to valid only.
    QContactFilter mergeMatchFilter = (originId.isEmpty() ? QContactInvalidFilter() : originIdFilter & syncTargetFilter);
    return mergeMatchFilter;
}

bool ContactBuilder::mergeLocalIntoImport(QContact &import, const QContact &local, bool *erase)
{
    // Always perform "clobber" save, never attempt to merge details.
    // This is because the "local" contact here will actually have the appropriate
    // sync target and origin Id already (due to our mergeMatchFilter() impl).
    import.setId(local.id()); // overwrite the local with the import contact.
    *erase = false; // we don't allow erasure.
    return true; // contact is modified / requires save.
}

// don't allow merging contacts in the import list.
bool ContactBuilder::mergeImportIntoImport(QContact &, QContact &, bool *erase)
{
    *erase = false;
    return false;
}

// don't allow de-duplication of contacts in the import list.
int ContactBuilder::previousDuplicateIndex(QList<QContact> &, QContact &, int)
{
    return -1;
}
