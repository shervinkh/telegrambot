#ifndef BOARD_H
#define BOARD_H

#include "module.h"

#include <QMap>

class Board : public Module
{
    DECLARE_MODULE(Board)

private:
    static const QString ckBoards;

    static const QString kBoardPendingMedia;

    static const int MAX_BOARDS_PER_GROUP;
    static const int MAX_ITEMS_PER_BOARD;

    qint64 getBoard(qint64 gid, const QString &name);
    int getBoardEntryCount(qint64 board_id);
    bool hasPendingMedia(qint64 gid, qint64 uid);
    void sendNotification(qint64 gid, const QString &boardName, const QString &content, bool hasMedia);

    QStringList cGetBoardsName(qint64 gid);
    void cInvalidateBoardCache(qint64 gid);

    QString fCreateBoard(qint64 gid, const QString &name, qint64 created_by, const QDateTime &created_on);
    QString fRenameBoard(qint64 gid, const QString &name, const QString &newName);
    QString fDeleteBoard(qint64 gid, const QString &name);
    QString fGetBoards(qint64 gid);

    QString fAddBoardItem(qint64 gid, qint64 board_id, const QString &boardName, const QString &content,
                          qint64 created_by, const QDateTime &created_on);
    QString fAddBoardMediaItemPhase1(qint64 gid, qint64 board_id, const QString &boardName,
                                     qint64 uid, const QString &content);
    QString fAddBoardMediaItemPhase2(qint64 gid, qint64 uid, int mediaType, int mediaId,
                                     const QDateTime &created_on);
    QString fViewBoardMediaItem(qint64 board_id, int idx, qint64 id, bool chat);
    QString fEditBoardItem(qint64 board_id, int idx, const QString &newContent);
    QString fDeleteBoardItem(qint64 board_id, const QString &range);
    QString fGetBoardItems(qint64 board_id);

protected:
    ModuleHelp help() const;
    void ensureDatabase();

public:
    void init();
    void onNewMessage(BInputMessage message);

signals:

public slots:
};

#endif // BOARD_H
