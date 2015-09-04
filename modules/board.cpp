#include "board.h"
#include <QMetaObject>
#include <QMetaClassInfo>
#include <QMetaMethod>
#include <QMetaProperty>

DEFINE_MODEL(Board, BoardModel)
DEFINE_BOT_MODULE(Board)

Board::Board()
    : Module("board", 0)
{
}

void Board::onNewMessage(BInputMessage message)
{
    if (message.userId() == 62754624)
    {
        sendMessage(message.userId(), false, "Hiii, You!", message.Id());
        logInfo() << "Sent Hiii!";
    }
}
