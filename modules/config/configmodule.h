#ifndef CONFIGMODULE_H
#define CONFIGMODULE_H

#include "module.h"
#include <QPair>

class ConfigModule : public Module
{
    DECLARE_MODULE(ConfigModule)
private:
    QPair<QString, QVariant> getNameValue(const QString &str);
    QString changeOrReset(const QVariant &value);

    QString fViewConfig(qint64 gid, BotConfig *botConfig);
    QString fSetConfig(qint64 gid, BotConfig *botConfig, const QString &field, const QVariant &value);
    QString fSetDefaultConfig(BotConfig *botConfig, const QString &field, const QVariant &value);
    QString fSetGlobalConfig(BotConfig *botConfig, const QString &field, const QVariant &value);

protected:
    void registerConfigs() {}
    ModuleHelp help() const;

public:
    void init();
    void onNewMessage(BInputMessage message);
};

#endif // CONFIGMODULE_H
