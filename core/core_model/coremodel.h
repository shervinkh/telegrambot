#ifndef COREMODEL_H
#define COREMODEL_H

#include <QObject>

class BotInterface;

class CoreModel : public QObject
{
private:
    BotInterface *mBotInterface;

    void ensureDatabase();

public:
    explicit CoreModel(BotInterface *botInterface, QObject *parent = 0);
    void init();
    void updateModelInfo(QObject *model);
};

#endif // COREMODEL_H
