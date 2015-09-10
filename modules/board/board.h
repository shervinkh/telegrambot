#ifndef BOARD_H
#define BOARD_H

#include "module.h"

#include <QMap>

class Board : public Module
{
    DECLARE_MODULE(Board)

private:
    qint64 getBoard(qint64 gid, const QString &name);
    QStringList getBoards(qint64 gid);

    QString fCreateBoard(qint64 gid, const QString &name, qint64 created_by, const QDateTime &created_on);
    QString fDeleteBoard(qint64 gid, const QString &name);
    QString fGetBoards(qint64 gid);

    QString fAddBoardItem(qint64 board_id, const QString &content, qint64 created_by, const QDateTime &created_on);
    QString fDeleteBoardItem(qint64 board_id, int idx);
    QString fGetBoardItems(qint64 board_id);

protected:
    void ensureDatabase();

public:
    void onNewMessage(BInputMessage message);

signals:

public slots:
};

#endif // BOARD_H
