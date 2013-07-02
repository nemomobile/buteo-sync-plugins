#ifndef CONTACTDETAILHANDLER_H
#define CONTACTDETAILHANDLER_H

#include <QVersitDocument>
#include <QVersitContactExporterDetailHandlerV2>
#include <QVersitProperty>
#include <QContact>
#include <QSet>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
QTM_USE_NAMESPACE
#else
using namespace QtVersit;
#endif

/*! \brief ContactDetailHandler class removes unused fields from vcard
 *
 */

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
                          const QVersitDocument & document, QSet<int> * processedFields,
                          QList<QVersitProperty> * toBeRemoved, QList<QVersitProperty> * toBeAdded);

};

#endif // CONTACTDETAILHANDLER_H
