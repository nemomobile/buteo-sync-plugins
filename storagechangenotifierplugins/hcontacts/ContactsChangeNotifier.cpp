#include "ContactsChangeNotifier.h"
#include "LogMacros.h"
#include <QList>

const QString DEFAULT_CONTACTS_MANAGER("tracker");

ContactsChangeNotifier::ContactsChangeNotifier() :
iDisabled(true)
{
    FUNCTION_CALL_TRACE;
    iManager = new QContactManager("org.nemomobile.contacts.sqlite");
}

ContactsChangeNotifier::~ContactsChangeNotifier()
{
    disable();
    delete iManager;
}

void ContactsChangeNotifier::enable()
{
    if(iManager && iDisabled)
    {
        QObject::connect(iManager, SIGNAL(contactsAdded(const QList<QContactLocalId>&)),
                         this, SLOT(onContactsAdded(const QList<QContactLocalId>&)));

        QObject::connect(iManager, SIGNAL(contactsRemoved(const QList<QContactLocalId>&)),
                         this, SLOT(onContactsRemoved(const QList<QContactLocalId>&)));

        QObject::connect(iManager, SIGNAL(contactsChanged(const QList<QContactLocalId>&)),
                         this, SLOT(onContactsChanged(const QList<QContactLocalId>&)));
        iDisabled = false;
    }
}

void ContactsChangeNotifier::onContactsAdded(const QList<QContactLocalId>& ids)
{
    FUNCTION_CALL_TRACE;
    if(ids.count())
    {
        QList<QContact> contacts = iManager->contacts(ids);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        foreach(QContact contact, contacts)
        {
            LOG_DEBUG("Added contact" << contact.displayLabel());
        }
#endif
        emit change();
    }
}

void ContactsChangeNotifier::onContactsRemoved(const QList<QContactLocalId>& ids)
{
    FUNCTION_CALL_TRACE;
    if(ids.count())
    {
        foreach(QContactLocalId id, ids)
        {
            LOG_DEBUG("Removed contact with id" << id);
        }
        emit change();
    }
}

void ContactsChangeNotifier::onContactsChanged(const QList<QContactLocalId>& ids)
{
    FUNCTION_CALL_TRACE;
    if(ids.count())
    {
        QList<QContact> contacts = iManager->contacts(ids);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        foreach(QContact contact, contacts)
        {
            LOG_DEBUG("Changed contact" << contact.displayLabel());
        }
#endif
        emit change();
    }
}

void ContactsChangeNotifier::disable()
{
    FUNCTION_CALL_TRACE;
    iDisabled = true;
    QObject::disconnect(iManager, 0, this, 0);
}
