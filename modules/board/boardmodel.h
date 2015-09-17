#ifndef BOARDMODEL_H
#define BOARDMODEL_H

#include "model.h"

class BoardModel : public QObject
{
    Q_OBJECT
    DECLARE_MODEL(BoardModel, board, 0, QDate(2015, 9, 11))
    DECLARE_MODEL_FIELD(qint64, gid)
    DECLARE_MODEL_FIELD(QString, name)
    DECLARE_MODEL_FIELD(qint64, created_by)
    DECLARE_MODEL_FIELD(QDateTime, created_on)
};

#endif // BOARDMODEL_H
