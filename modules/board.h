#ifndef BOARD_H
#define BOARD_H

#include "module.h"

class Board : public Module
{
    Q_OBJECT
public:
    explicit Board();
    void onNewMessage(BInputMessage message);

signals:

public slots:
};

#endif // BOARD_H
