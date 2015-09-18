#ifndef SUBSCRIPTIONMODEL
#define SUBSCRIPTIONMODEL

#include "model.h"

class SubscriptionModel : public QObject
{
    Q_OBJECT
    DECLARE_MODEL(SubscriptionModel, subscription, 0, QDate(2015, 9, 18))
    DECLARE_MODEL_FIELD(qint64, gid)
    DECLARE_MODEL_FIELD(qint64, uid)
    DECLARE_MODEL_FIELD(QDateTime, subscribed_on)
};

#endif // SUBSCRIPTIONMODEL

