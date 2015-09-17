#ifndef INSTALLEDMODELMODEL
#define INSTALLEDMODELMODEL

#include "model.h"

class InstalledModelModel : public QObject
{
    Q_OBJECT
    DECLARE_MODEL(InstalledModelModel, installed_model, 0, QDate(2015, 9, 17))
    DECLARE_MODEL_FIELD(QString, section)
    DECLARE_MODEL_FIELD(QString, name)
    DECLARE_MODEL_FIELD(qint64, installed_version)
    DECLARE_MODEL_FIELD(QDateTime, version_date)
    DECLARE_MODEL_FIELD(QDateTime, installed_date)
};

#endif // INSTALLEDMODELMODEL

