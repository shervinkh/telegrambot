#ifndef CONFIGMODULE_H
#define CONFIGMODULE_H

#include "module.h"
#include <QPair>

class ConfigModule : public Module
{
private:
    QPair<QString, QVariant> getNameValue(const QString &str);
    QString changeOrReset(const QVariant &value);

    QString fViewConfig(qint64 gid, BotConfig *botConfig);
    QString fSetConfig(qint64 gid, BotConfig *botConfig, const QString &field,
                       const QVariant &value, bool authorized);
    QString fSetDefaultConfig(BotConfig *botConfig, const QString &field,
                              const QVariant &value, bool authorized);
    QString fSetGlobalConfig(BotConfig *botConfig, const QString &field,
                             const QVariant &value, bool authorized);

protected:
    void registerConfigs() {}
    ModuleHelp help() const;

public:
    ConfigModule();
    void init();
    void onNewMessage(BInputMessage message);
};

#endif // CONFIGMODULE_H
