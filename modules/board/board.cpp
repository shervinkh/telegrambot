#include "board.h"
#include "boardmodel.h"
#include "bot.h"

DEFINE_MODULE(Board)
DEFINE_MODEL(BoardModel)

Board::Board()
    : Module("board", 0)
{
    registerModel(MODEL(BoardModel));
}

void Board::ensureDatabase()
{
    mBot->executeDatabaseQuery("CREATE TABLE IF NOT EXISTS bot_modules_board_boards ("
                               "    id bigserial PRIMARY KEY,"
                               "    name text NOT NULL,"
                               "    gid bigint NOT NULL,"
                               "    created_by bigint,"
                               "    created_on timestamp with time zone"
                               ")");
    mBot->executeDatabaseQuery("CREATE UNIQUE INDEX unique_name_gid_index "
                               "ON bot_modules_board_boards(name, gid)");

    mBot->executeDatabaseQuery("CREATE TABLE IF NOT EXISTS bot_modules_board_board_items ("
                               "    id bigserial PRIMARY KEY,"
                               "    board_id bigint REFERENCES bot_modules_board_boards(id) ON DELETE CASCADE ON UPDATE CASCADE,"
                               "    content text,"
                               "    created_by bigint,"
                               "    created_on timestamp with time zone"
                               ")");
    mBot->executeDatabaseQuery("CREATE INDEX board_id_index "
                               "ON bot_modules_board_board_items(board_id)");

    BoardModel::PointerType board = MODEL(BoardModel)->newInstance();
    board->setProperty("gid", 1155);
    board->setProperty("name", "sup2");
    board->setProperty("id", 3);
    logInfo() << "saving!";
    board->save();
    logInfo() << "saved!";
}

void Board::onNewMessage(BInputMessage message)
{
    if (message.userId() == 62754624)
    {
        mBot->sendMessage(message.userId(), false, "Hiii, You!", message.Id());
        logInfo() << "Sent Hiii!";
    }
}
