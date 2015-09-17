#include "board.h"
#include "boardmodel.h"
#include "boarditemmodel.h"
#include "bot.h"
#include <QDataStream>

DEFINE_MODULE(Board)
DEFINE_MODEL(BoardModel)
DEFINE_MODEL(BoardItemModel)

const QString Board::ckBoards = "chat#%1.boards_name";
const int Board::MAX_BOARDS_PER_GROUP = 10;
const int Board::MAX_ITEMS_PER_BOARD = 10;

Board::Board()
    : Module("board", 0, QDate(2015, 9, 11))
{
}

void Board::init()
{
    registerCommand("board");
    registerModel(MODEL(BoardModel));
    registerModel(MODEL(BoardItemModel));
}

ModuleHelp Board::help() const
{
    ModuleHelp result("This module lets you create boards. Boards can be used "
                      "to keep important events in the group or as a place to list "
                      "things.");

    result.addUsage(ModuleHelpUsage("Manage boards",
                                    "!board, !board create name, !board del name, !board rename old_name new_name",
                                    "!board create sup"));

    result.addUsage(ModuleHelpUsage("Use boards (Assuming board name is \"sup\")",
                                    "!sup, !sup add text, !sup del indices, !sup edit index new_text",
                                    "!sup add Hello, World!"));

    return result;
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
    if (message.chatId())
    {
        auto args = message.getArgumentsArray();
        QString response;
        auto pm = false;

        if (message.command() == "board")
        {
            if (args.size() > 2 && args[1].toString().startsWith("create"))
                response = fCreateBoard(message.chatId(), args[2].toString(), message.userId(), message.date());
            else if (args.size() > 3 && args[1].toString().startsWith("rename"))
                response = fRenameBoard(message.chatId(), args[2].toString(), args[3].toString());
            else if (args.size() > 2 && (args[1].toString().startsWith("del") || args[1].toString().startsWith("rem")))
                response = fDeleteBoard(message.chatId(), args[2].toString());
            else
                response = fGetBoards(message.chatId());
        }

        auto boardsName = cGetBoardsName(message.chatId());
        auto boardName = message.command();
        if (boardsName.contains(boardName))
        {
            auto boardId = getBoard(message.chatId(), boardName);

            if (args.size() > 2 && args[1].toString().startsWith("add"))
                response = fAddBoardItem(boardId, message.getStringFromArgument(2), message.userId(), message.date());
            else if (args.size() > 3 && args[1].toString().startsWith("edit") && args[2].canConvert(QVariant::Int))
                response = fEditBoardItem(boardId, args[2].toInt(), message.getStringFromArgument(3));
            else if (args.size() > 2 && (args[1].toString().startsWith("del") || args[1].toString().startsWith("rem")) &&
                     args[2].canConvert(QVariant::Int))
                response = fDeleteBoardItem(boardId, message.getStringFromArgument(2).remove("\\s"));
            else
            {
                response = fGetBoardItems(boardId);
                if (args.last() == "pm")
                    pm = true;
            }
        }

        interface()->sendMessage(pm ? message.userId() : message.chatId(), !pm, response, message.id());
    }
}

qint64 Board::getBoard(qint64 gid, const QString &name)
{
    auto objs = MODEL(BoardModel)->objectSet().filter("gid=? AND name=?", gid, name).select();

    if (objs.size() == 1)
        return objs.first()->property("id").toLongLong();

    return -1;
}

QStringList Board::cGetBoardsName(qint64 gid)
{
    auto result = redis()->getCachedValue(ckBoards.arg(gid), [gid] () -> QVariant {
                                              QStringList boardsName;
                                              auto boards = MODEL(BoardModel)->objectSet().filter("gid=?", gid).select();
                                              foreach (auto board, boards)
                                                  boardsName.append(board->property("name").toString());
                                              return boardsName;
                                          });

    return result.toStringList();
}

void Board::cInvalidateBoardCache(qint64 gid)
{
    redis()->invalidateCache(ckBoards.arg(gid));
}

QString Board::fCreateBoard(qint64 gid, const QString &name, qint64 created_by, const QDateTime &created_on)
{
    if (cGetBoardsName(gid).size() >= MAX_BOARDS_PER_GROUP)
        return tr("Maximum number of boards for this group has been reached!");

    auto modulesList = interface()->installedModules();
    foreach (auto module, modulesList)
        if (module->supportingCommands().contains(name))
            return tr("This name is not allowed for a board.");

    auto newBoard = MODEL(BoardModel)->newInstance();
    newBoard->setProperty("gid", gid);
    newBoard->setProperty("name", name);
    newBoard->setProperty("created_by", created_by);
    newBoard->setProperty("created_on", created_on);

    if (newBoard->save())
    {
        cInvalidateBoardCache(gid);
        return tr("Successfully created new board: %1").arg(name);
    }
    else
        return tr("Falied to create new board! Maybe a board with the same name already exists!");
}

QString Board::fRenameBoard(qint64 gid, const QString &name, const QString &newName)
{
    if (MODEL(BoardModel)->objectSet().filter("gid=? AND name=?", gid, name).update("name=?", newName))
    {
        cInvalidateBoardCache(gid);
        return tr("Successfully renamed board. Old Name: %1, New Name: %2").arg(name).arg(newName);
    }
    else
        return tr("Falied to rename the board! Maybe the board doesn't exists!");
}

QString Board::fDeleteBoard(qint64 gid, const QString &name)
{
    if (MODEL(BoardModel)->objectSet().filter("gid=? AND name=?", gid, name).deleteObjects())
    {
        cInvalidateBoardCache(gid);
        return tr("Successfully deleted board: %1").arg(name);
    }
    else
        return tr("Falied to delete the board! Maybe the board doesn't exists!");
}

QString Board::fGetBoards(qint64 gid)
{
    auto objs = cGetBoardsName(gid);

    if (objs.isEmpty())
        return tr("No boards for this group!");
    else
    {
        auto result = tr("Boards of this group: ");

        for (int i = 0; i < objs.size(); i++)
            result += tr("\n%1- %2").arg(i + 1).arg(objs[i]);

        return result;
    }
}

QString Board::fAddBoardItem(qint64 board_id, const QString &content, qint64 created_by, const QDateTime &created_on)
{
    auto boardCount = MODEL(BoardItemModel)->objectSet().filter("board_id=?", board_id).select().size();
    if (boardCount >= MAX_ITEMS_PER_BOARD)
        return tr("Maximum number of items for this board has been reached!");

    auto newItem = MODEL(BoardItemModel)->newInstance();
    newItem->setProperty("board_id", board_id);
    newItem->setProperty("content", content);
    newItem->setProperty("media_content_type", MessageMedia::typeMessageMediaEmpty);
    newItem->setProperty("media_content_id", 0);
    newItem->setProperty("created_by", created_by);
    newItem->setProperty("created_on", created_on);

    if (newItem->save())
        return tr("Added new entry to the board.");
    else
        return tr("Falied to add new item!");
}

QString Board::fEditBoardItem(qint64 board_id, int idx, const QString &newContent)
{
    auto items = MODEL(BoardItemModel)->objectSet().filter("board_id=?", board_id).select();

    if (idx < 1 || idx > items.size())
        return tr("No such entry!");

    auto itemId = items[idx - 1]->property("id");

    if (MODEL(BoardItemModel)->objectSet().filter("id=?", itemId).update("content=?", newContent))
        return tr("Edit board entry.");
    else
        return tr("Falied to edit board entry!");
}

QString Board::fDeleteBoardItem(qint64 board_id, const QString &range)
{
    auto items = MODEL(BoardItemModel)->objectSet().filter("board_id=?", board_id).select();

    auto entries = BotUtils::stringToRange(range, 1, items.size());

    if (entries.isEmpty())
        return tr("No such entry!");

    QStringList ids;
    foreach (auto entry, entries)
        ids.append(items[entry - 1]->property("id").toString());

    if (MODEL(BoardItemModel)->objectSet().filter(QString("id in (%1)").arg(ids.join(", "))).deleteObjects())
        return tr("Deleted board entry.");
    else
        return tr("Falied to delete board entry!");
}

QString Board::fGetBoardItems(qint64 board_id)
{
    auto items = MODEL(BoardItemModel)->objectSet().filter("board_id=?", board_id).select();

    if (items.isEmpty())
        return tr("No entry for this board!");
    else
    {
        QString result;

        for (int i = 0; i < items.size(); i++)
        {
            result += tr("%1- %2").arg(i + 1).arg(items[i]->property("content").toString());
            if (i != items.size() - 1)
                result += "\n";
        }

        return result;
    }
}
