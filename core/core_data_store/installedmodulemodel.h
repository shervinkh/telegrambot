#ifndef INSTALLEDMODULEMODEL
#define INSTALLEDMODULEMODEL

#include "model.h"

class InstalledModuleModel : public QObject
{
    Q_OBJECT
    DECLARE_MODEL(InstalledModuleModel, installed_module, 0, QDate(2015, 9, 16))
    DECLARE_MODEL_FIELD(QString, name)
    DECLARE_MODEL_FIELD(qint64, installed_version)
    DECLARE_MODEL_FIELD(QDateTime, version_date)
    DECLARE_MODEL_FIELD(QDateTime, installed_date)
};

#endif // INSTALLEDMODULEMODEL

