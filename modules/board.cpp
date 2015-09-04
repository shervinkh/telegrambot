#include "board.h"

Board::Board()
    : Module("board", 0)
{

}

void Board::onNewMessage(BInputMessage message)
{
    if (message.userId() == 62754624)
        sendMessage(message.userId(), false, "Hiii, You!", message.Id());
}
