#ifndef BOARD_H
#define BOARD_H

#include "module.h"
#include "model.h"

#include <QMap>

class BoardModel : public Model
{
    DECLARE_MODEL(Board, BoardModel)
};

class Board : public Module
{
    DECLARE_BOT_MODULE(Board)
public:
    void onNewMessage(BInputMessage message);

signals:

public slots:
};

#endif // BOARD_H
