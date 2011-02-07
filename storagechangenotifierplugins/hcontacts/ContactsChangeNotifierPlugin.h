#ifndef CONTACTSCHANGENOTIFIERPLUGIN_H
#define CONTACTSCHANGENOTIFIERPLUGIN_H

#include "StorageChangeNotifierPlugin.h"

class ContactsChangeNotifier;

class ContactsChangeNotifierPlugin : public Buteo::StorageChangeNotifierPlugin
{
    Q_OBJECT

public:
    /*! \brief constructor
     * see StorageChangeNotifierPlugin
     */
    ContactsChangeNotifierPlugin(const QString& aStorageName);

    /*! \brief destructor
     */
    ~ContactsChangeNotifierPlugin();

    /*! \brief see StorageChangeNotifierPlugin::name
     */
    QString name() const;

    /*! \brief see StorageChangeNotifierPlugin::hasChanges
     */
    bool hasChanges() const;

    /*! \brief see StorageChangeNotifierPlugin::changesReceived
     */
    void changesReceived();

    /*! \brief see StorageChangeNotifierPlugin::enable
     */
    void enable();

    /*! \brief see StorageChangeNotifierPlugin::disable
     */
    void disable(bool disableAfterNextChange = false);

private Q_SLOTS:
    /*! \brief handles a change notification from contacts notifier
     */
    void onChange();

private:
    ContactsChangeNotifier* icontactsChangeNotifier;
    bool ihasChanges;
    bool iDisableLater;
};

#endif
