#ifndef BOARD_H
#define BOARD_H

#include "module.h"

#include <QMap>

class Board : public Module
{
    DECLARE_MODULE(Board)

private:
    void createBoard(qint64 gid, const QString &name);
    void deleteBoard(qint64 gid, const QString &name);
    QList<QString> getBoards(qint64 gid);
    qint64 getBoard(qint64 gid, const QString &name);

    void addBoardItem(qint64 board_id, const QString &content);
    void deleteBoardItem(qint64 board_id, int idx);
    QList<QString> getBoardItems(qint64 board_id);

protected:
    void ensureDatabase();

public:
    void onNewMessage(BInputMessage message);

signals:

public slots:
};

#endif // BOARD_H
