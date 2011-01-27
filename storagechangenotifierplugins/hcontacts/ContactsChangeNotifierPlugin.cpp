#include "ContactsChangeNotifierPlugin.h"
#include "ContactsChangeNotifier.h"
#include "LogMacros.h"
#include <QTimer>

using namespace Buteo;

extern "C" StorageChangeNotifierPlugin* createPlugin(const QString& aStorageName)
{
    return new ContactsChangeNotifierPlugin(aStorageName);
}

extern "C" void destroyPlugin(StorageChangeNotifierPlugin* plugin)
{
    delete plugin;
}

ContactsChangeNotifierPlugin::ContactsChangeNotifierPlugin(const QString& aStorageName) :
StorageChangeNotifierPlugin(aStorageName),
m_hasChanges(false)
{
    FUNCTION_CALL_TRACE;
    m_contactsChangeNotifier = new ContactsChangeNotifier;
    m_contactsChangeNotifier->enable();
    QObject::connect(m_contactsChangeNotifier, SIGNAL(change()),
                     this, SLOT(onChange()));
}

ContactsChangeNotifierPlugin::~ContactsChangeNotifierPlugin()
{
    FUNCTION_CALL_TRACE;
    delete m_contactsChangeNotifier;
}

QString ContactsChangeNotifierPlugin::name() const
{
    FUNCTION_CALL_TRACE;
    return iStorageName;
}

bool ContactsChangeNotifierPlugin::hasChanges() const
{
    FUNCTION_CALL_TRACE;
    return m_hasChanges;
}

void ContactsChangeNotifierPlugin::changesReceived()
{
    FUNCTION_CALL_TRACE;
    m_hasChanges = false;
}

void ContactsChangeNotifierPlugin::onChange()
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("Change in contacts detected");
    m_hasChanges = true;
    emit storageChange();
}
