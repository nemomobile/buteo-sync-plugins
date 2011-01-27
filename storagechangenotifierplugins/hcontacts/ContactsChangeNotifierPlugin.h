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

private Q_SLOTS:
    /*! \brief handles a change notification from contacts notifier
     */
    void onChange();

private:
    ContactsChangeNotifier* m_contactsChangeNotifier;
    bool m_hasChanges;
};

#endif
