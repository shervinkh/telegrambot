#include "board.h"
#include "boardmodel.h"
#include "boarditemmodel.h"
#include "bot.h"

DEFINE_MODULE(Board)
DEFINE_MODEL(BoardModel)
DEFINE_MODEL(BoardItemModel)

Board::Board()
    : Module("board", 0)
{
    registerModel(MODEL(BoardModel));
    registerModel(MODEL(BoardItemModel));
}

void Board::ensureDatabase()
{
    interface()->executeDatabaseQuery("CREATE TABLE IF NOT EXISTS bot_modules_board_boards ("
                               "    id bigserial PRIMARY KEY,"
                               "    name text NOT NULL,"
                               "    gid bigint NOT NULL,"
                               "    created_by bigint,"
                               "    created_on timestamp with time zone"
                               ")");
    interface()->executeDatabaseQuery("CREATE UNIQUE INDEX unique_name_gid_index "
                               "ON bot_modules_board_boards(name, gid)");

    interface()->executeDatabaseQuery("CREATE TABLE IF NOT EXISTS bot_modules_board_board_items ("
                               "    id bigserial PRIMARY KEY,"
                               "    board_id bigint REFERENCES bot_modules_board_boards(id) ON DELETE CASCADE ON UPDATE CASCADE,"
                               "    content text,"
                               "    media_content_type bigint,"
                               "    media_content_id bigint,"
                               "    created_by bigint,"
                               "    created_on timestamp with time zone"
                               ")");
    interface()->executeDatabaseQuery("CREATE INDEX board_id_index "
                               "ON bot_modules_board_board_items(board_id)");
}

void Board::onNewMessage(BInputMessage message)
{
    auto args = message.getArgumentsArray();

    QString response;

    if (message.chatId() && args.size() > 0 && (args[0] == "!board" || args[0] == "/board"))
    {
        if (args.size() > 2 && args[1] == "create")
            response = fCreateBoard(message.chatId(), args[2], message.userId(), message.date());
        else if (args.size() > 2 && args[1].startsWith("del"))
            response = fDeleteBoard(message.chatId(), args[2]);
        else
            response = fGetBoards(message.chatId());
    }

    interface()->sendMessage(message.chatId(), true, response, message.id());
}

qint64 Board::getBoard(qint64 gid, const QString &name)
{
    auto objs = MODEL(BoardModel)->objectSet().filter("gid=? AND name=?", gid, name).select();

    if (objs.size() == 1)
        return objs.first()->property("id").toLongLong();

    return -1;
}

QStringList Board::getBoards(qint64 gid)
{
}

QString Board::fCreateBoard(qint64 gid, const QString &name, qint64 created_by, const QDateTime &created_on)
{
    auto newBoard = MODEL(BoardModel)->newInstance();
    newBoard->setProperty("gid", gid);
    newBoard->setProperty("name", name);
    newBoard->setProperty("created_by", created_by);
    newBoard->setProperty("created_on", created_on);

    if (newBoard->save())
        return tr("Successfully created new board: %1").arg(name);
    else
        return tr("Falied to create new board! Maybe a board with the same name already exists!");
}

QString Board::fDeleteBoard(qint64 gid, const QString &name)
{
    if (MODEL(BoardModel)->objectSet().filter("gid=? AND name=?", gid, name).deleteObjects())
        return tr("Successfully deleted board: %1").arg(name);
    else
        return tr("Falied to delete the board! Maybe the board doesn't exists!");
}

QString Board::fGetBoards(qint64 gid)
{
    auto objs = MODEL(BoardModel)->objectSet().filter("gid=?", gid).select();

    if (objs.isEmpty())
        return tr("No boards for this group!");
    else
    {
        QString result = tr("Boards of this group: ");

        for (int i = 0; i < objs.size(); i++)
            result += tr("\n%1- %2").arg(i + 1).arg(objs[i]->property("name").toString());

        return result;
    }
}

//QString addBoardItem(qint64 board_id, const QString &content);
//QString deleteBoardItem(qint64 board_id, int idx);
//QString getBoardItems(qint64 board_id);
