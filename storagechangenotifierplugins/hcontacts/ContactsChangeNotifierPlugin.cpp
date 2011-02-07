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
ihasChanges(false),
iDisableLater(false)
{
    FUNCTION_CALL_TRACE;
    icontactsChangeNotifier = new ContactsChangeNotifier;
    QObject::connect(icontactsChangeNotifier, SIGNAL(change()),
                     this, SLOT(onChange()));
}

ContactsChangeNotifierPlugin::~ContactsChangeNotifierPlugin()
{
    FUNCTION_CALL_TRACE;
    delete icontactsChangeNotifier;
}

QString ContactsChangeNotifierPlugin::name() const
{
    FUNCTION_CALL_TRACE;
    return iStorageName;
}

bool ContactsChangeNotifierPlugin::hasChanges() const
{
    FUNCTION_CALL_TRACE;
    return ihasChanges;
}

void ContactsChangeNotifierPlugin::changesReceived()
{
    FUNCTION_CALL_TRACE;
    ihasChanges = false;
}

void ContactsChangeNotifierPlugin::onChange()
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("Change in contacts detected");
    ihasChanges = true;
    if(iDisableLater)
    {
        icontactsChangeNotifier->disable();
    }
    else
    {
        emit storageChange();
    }
}

void ContactsChangeNotifierPlugin::enable()
{
    FUNCTION_CALL_TRACE;
    icontactsChangeNotifier->enable();
    iDisableLater = false;
}

void ContactsChangeNotifierPlugin::disable(bool disableAfterNextChange)
{
    FUNCTION_CALL_TRACE;
    if(disableAfterNextChange)
    {
        iDisableLater = true;
    }
    else
    {
        icontactsChangeNotifier->disable();
    }
}
