#ifndef CONTACTDETAILHANDLER_H
#define CONTACTDETAILHANDLER_H

#include <QVersitContactExporterDetailHandlerV2>
#include <qversitdocument.h>
#include <qversitproperty.h>
#include <qcontact.h>
#include <QObject>
#include <QSet>
#include <QString>

/*! \brief ContactDetailHandler class removes unused fields from vcard
 *
 */

QTM_USE_NAMESPACE;


class ContactDetailHandler : public QVersitContactExporterDetailHandlerV2
{
public:
    ContactDetailHandler();

    /*! \brief Modifies document produced from contact.
     *   Currently only removes ringtone field.
     * @param contact Contact from which the document was produced
     * @param document Document which is modified
     */
    virtual void contactProcessed (const QContact & contact, QVersitDocument * document);

    /*! \brief Not used/implemented.
     */
    virtual void detailProcessed (const QContact & contact, const QContactDetail & detail,
                                  const QVersitDocument & document, QSet<QString> * processedFields,
                                  QList<QVersitProperty> * toBeRemoved, QList<QVersitProperty> * toBeAdded);

};

#endif // CONTACTDETAILHANDLER_H
