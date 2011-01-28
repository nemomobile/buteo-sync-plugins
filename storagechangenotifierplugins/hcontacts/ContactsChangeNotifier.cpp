#include "ContactsChangeNotifier.h"
#include "LogMacros.h"
#include <QList>

const QString DEFAULT_CONTACTS_MANAGER("tracker");

ContactsChangeNotifier::ContactsChangeNotifier()
{
    FUNCTION_CALL_TRACE;
    iManager = 0;
    QStringList availableManagers = QContactManager::availableManagers();
    if(availableManagers.contains(DEFAULT_CONTACTS_MANAGER))
    {
        iManager = new QContactManager(DEFAULT_CONTACTS_MANAGER);
    }
    else
    {
        LOG_WARNING("Couldn't create a contacts manager");
    }
}

ContactsChangeNotifier::~ContactsChangeNotifier()
{
    disable();
    delete iManager;
}

void ContactsChangeNotifier::enable()
{
    if(iManager)
    {
        QObject::connect(iManager, SIGNAL(contactsAdded(const QList<QContactLocalId>&)),
                         this, SLOT(onContactsAdded(const QList<QContactLocalId>&)));

        QObject::connect(iManager, SIGNAL(contactsRemoved(const QList<QContactLocalId>&)),
                         this, SLOT(onContactsRemoved(const QList<QContactLocalId>&)));

        QObject::connect(iManager, SIGNAL(contactsChanged(const QList<QContactLocalId>&)),
                         this, SLOT(onContactsChanged(const QList<QContactLocalId>&)));
    }
}

void ContactsChangeNotifier::onContactsAdded(const QList<QContactLocalId>& ids)
{
    FUNCTION_CALL_TRACE;
    if(ids.count())
    { 
        QList<QContact> contacts = iManager->contacts(ids);
        foreach(QContact contact, contacts)
        {
            LOG_DEBUG("Added contact" << contact.displayLabel());
        }
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
        foreach(QContact contact, contacts)
        {
            LOG_DEBUG("Changed contact" << contact.displayLabel());
        }
        emit change();
    }
}

void ContactsChangeNotifier::disable()
{
    this->disconnect();
    QObject::disconnect(iManager, 0, this, 0);
}
