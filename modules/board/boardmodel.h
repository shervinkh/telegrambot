#ifndef BOARDMODEL_H
#define BOARDMODEL_H

#include "model.h"

class BoardModel : public QObject
{
    Q_OBJECT
    DECLARE_MODEL(BoardModel, board)
    DECLARE_MODEL_FIELD(qint64, id)
    DECLARE_MODEL_FIELD(qint64, gid)
    DECLARE_MODEL_FIELD(QString, name)
};

#endif // BOARDMODEL_H
