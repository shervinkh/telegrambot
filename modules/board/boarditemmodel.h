#ifndef BOARDITEMMODEL_H
#define BOARDITEMMODEL_H

#include "model.h"

class BoardItemModel : public QObject
{
    Q_OBJECT
    DECLARE_MODEL(BoardItemModel, board_item)
    DECLARE_MODEL_FIELD(qint64, board_id)
    DECLARE_MODEL_FIELD(QString, content)
    DECLARE_MODEL_FIELD(qint64, media_content_type)
    DECLARE_MODEL_FIELD(qint64, media_content_id)
    DECLARE_MODEL_FIELD(qint64, created_by)
    DECLARE_MODEL_FIELD(QDateTime, created_on)
};

#endif // BOARDITEMMODEL_H

