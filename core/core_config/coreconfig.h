#ifndef CORECONFIG_H
#define CORECONFIG_H

#include <QObject>
#include <QMap>

class BotInterface;
class BotConfig;

class CoreConfig : public QObject
{
private:
    BotInterface *mBotInterface;

    QMap<QString, BotConfig *> mRegisteredConfigs;

public:
    explicit CoreConfig(BotInterface *botInterface, QObject *parent = 0);

    void registerConfig(BotConfig *botConfig);
    BotConfig *config(const QString &name);
};

#endif // CORECONFIG_H
