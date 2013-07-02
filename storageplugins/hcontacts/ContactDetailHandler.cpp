#include "ContactDetailHandler.h"

static const QString FIELD_SOUND("SOUND");

ContactDetailHandler::ContactDetailHandler()
{
}

void ContactDetailHandler::contactProcessed(const QContact &contact, QVersitDocument *document)
{
    Q_UNUSED(contact);
    // Currently we only need to remove SOUND field from the vcard.
    // This is because we don't want to transfer big MP3 files via
    // BT for example.
    document->removeProperties(FIELD_SOUND);
}

void ContactDetailHandler::detailProcessed(const QContact &contact, const QContactDetail &detail,
                                           const QVersitDocument &document, QSet<int> *processedFields,
                                           QList<QVersitProperty> *toBeRemoved, QList<QVersitProperty> *toBeAdded)
{
    Q_UNUSED(contact);
    Q_UNUSED(detail);
    Q_UNUSED(document);
    Q_UNUSED(processedFields);
    Q_UNUSED(toBeRemoved);
    Q_UNUSED(toBeAdded);
}
