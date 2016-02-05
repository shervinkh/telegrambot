#ifndef COREMODULE_H
#define COREMODULE_H

#include <QObject>
#include <QMap>

class BotInterface;
class Model;
class Module;

class CoreModule : public QObject
{
private:
    BotInterface *mBotInterface;

    QMap<QString, Model *> mRegisteredModels;

    void registerModels();

public:
    explicit CoreModule(BotInterface *botInterface, QObject *parent = 0);
    void updateModuleInfo(Module *module);
};

#endif // COREMODULE_H
