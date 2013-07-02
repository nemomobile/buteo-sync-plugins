#ifndef CONTACTSCHANGENOTIFIER_H
#define CONTACTSCHANGENOTIFIER_H

#include <QObject>
#include <QContactManager>
#include <QList>

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
#include <QContactId>
using namespace QtContacts;
#define QContactLocalId QContactId
#else
QTM_USE_NAMESPACE;
#endif

class ContactsChangeNotifier : public QObject
{
    Q_OBJECT

public:
    /*! \brief constructor
     */
    ContactsChangeNotifier();

    /*! \brief constructor
     */
    ~ContactsChangeNotifier();

    /*! \brief start listening to changes from QContactManager
     */
    void enable();

    /*! \brief stop listening to changes from QContactManager
     */
    void disable();

Q_SIGNALS:
    /*! emit this signal to notify a change in contacts backend
     */
    void change();

private Q_SLOTS:
    void onContactsAdded(const QList<QContactLocalId>& ids);
    void onContactsRemoved(const QList<QContactLocalId>& ids);
    void onContactsChanged(const QList<QContactLocalId>& ids);

private:
    QContactManager* iManager;
    bool iDisabled;
};

#endif
