#ifndef COREMODEL_H
#define COREMODEL_H

#include <QObject>
#include <QMap>

class BotInterface;
class Model;

class CoreModel : public QObject
{
private:
    BotInterface *mBotInterface;

    QMap<QString, Model *> mRegisteredModels;

    void registerModels();
    void ensureDatabase();

public:
    explicit CoreModel(BotInterface *botInterface, QObject *parent = 0);
    void init();
    void updateModelInfo(Model *model);

    void registerModel(Model *model);
    Model *model(const QString &section, const QString &name);
};

#endif // COREMODEL_H
