#ifndef CONTACTSCHANGENOTIFIER_H
#define CONTACTSCHANGENOTIFIER_H

#include <QObject>
#include <QContactManager>
#include <QList>

QTM_USE_NAMESPACE;

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
