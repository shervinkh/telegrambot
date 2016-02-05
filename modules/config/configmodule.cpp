#include "configmodule.h"
#include "botinterface.h"

ConfigModule::ConfigModule()
    : Module("config", 0, QDate(2015, 10, 2))
{

}

void ConfigModule::init()
{
    registerCommand("config");
}

ModuleHelp ConfigModule::help() const
{
    ModuleHelp result("This module shows and lets you change configurations for bot modules.");

    result.addUsage(ModuleHelpUsage("View one configuration",
                                    "!config config_name",
                                    "!config modules_board"));

    result.addUsage(ModuleHelpUsage("Change a config value (Admin only)",
                                    "!config config_name field=value",
                                    "!config modules_board enabled=true"));

    result.addUsage(ModuleHelpUsage("Reset a config value to default (Admin only)",
                                    "!config config_name field reset",
                                    "!config modules_board enabled reset"));

    result.addUsage(ModuleHelpUsage("Change a config default value (Superuser only)",
                                    "!config config_name default field=value",
                                    "!config modules_board default enabled=true"));

    result.addUsage(ModuleHelpUsage("Change a global config value (Superuser only)",
                                    "!config config_name global field=value",
                                    "!config modules_board global enabled_global=true"));

    return result;
}

void ConfigModule::onNewMessage(BInputMessage message)
{
    QString response;

    if (message.command() == "config")
    {
        auto args = message.getArgumentsArray();

        if (args.size() > 1)
        {
            auto conf = interface()->config(args[1].toString());

            if (conf)
            {
                if (args.size() > 3 && args[2].toString().trimmed() == "default")
                {
                    auto confVals = getNameValue(message.getStringFromArgument(3));
                    response = fSetDefaultConfig(conf, confVals.first, confVals.second, message.isSuperuser());
                }
                else if (args.size() > 3 && args[2].toString().trimmed() == "global")
                {
                    auto confVals = getNameValue(message.getStringFromArgument(3));
                    response = fSetGlobalConfig(conf, confVals.first, confVals.second, message.isSuperuser());
                }
                else if (args.size() > 2 && message.chatId())
                {
                    auto confVals = getNameValue(message.getStringFromArgument(2));
                    response = fSetConfig(message.chatId(), conf, confVals.first, confVals.second, message.isAdmin());
                }
                else if (message.chatId())
                    response = fViewConfig(message.chatId(), conf);
            }
        }

    }

    auto pm = message.isPrivate();
    interface()->sendMessage(pm ? message.userId() : message.chatId(), !pm, response, message.id());
}

QPair<QString, QVariant> ConfigModule::getNameValue(const QString &str)
{
    auto strs = str.split('=');

    if (strs.size() == 2)
        return QPair<QString, QVariant>(strs[0].trimmed(), strs[1].trimmed());
    else
    {
        auto args = str.split(' ', QString::SkipEmptyParts);

        if (args.size() == 2 && args[1].toLower() == "reset")
            return QPair<QString, QVariant>(args[0].trimmed(), QVariant());
    }

    return QPair<QString, QVariant>();
}

QString ConfigModule::changeOrReset(const QVariant &value)
{
    return value.isNull() ? "reset" : "change";
}

QString ConfigModule::fViewConfig(qint64 gid, BotConfig *botConfig)
{
    auto result = tr("Config %1:").arg(botConfig->name());

    auto configs = botConfig->getConfig(gid);
    foreach (auto configName, configs.keys())
    {
        auto globalString = configs[configName].isGlobal() ? " (Global)" : "";
        result += QString("\n%1 (%2)%3: %4").arg(configName).arg(configs[configName].typeString())
                .arg(globalString).arg(configs[configName].value().toString());
    }

    return result;
}

QString ConfigModule::fSetConfig(qint64 gid, BotConfig *botConfig, const QString &field,
                                 const QVariant &value, bool authorized)
{
    if (!authorized)
        return tr("You are not authorized to do this");

    if (botConfig->setConfig(gid, 0, field, value))
        return tr("Config value %1 was successful.").arg(changeOrReset(value));
    else
        return tr("Failed to %1 config value!").arg(changeOrReset(value));
}

QString ConfigModule::fSetDefaultConfig(BotConfig *botConfig, const QString &field,
                                        const QVariant &value, bool authorized)
{
    if (!authorized)
        return tr("You are not authorized to do this");

    if (botConfig->setDefaultConfig(field, value))
        return tr("Default config value %1 was successful.").arg(changeOrReset(value));
    else
        return tr("Failed to %1 default config value!").arg(changeOrReset(value));
}

QString ConfigModule::fSetGlobalConfig(BotConfig *botConfig, const QString &field,
                                       const QVariant &value, bool authorized)
{
    if (!authorized)
        return tr("You are not authorized to do this");

    if (botConfig->setGlobalConfig(field, value))
        return tr("Global config value %1 was successful.").arg(changeOrReset(value));
    else
        return tr("Failed to %1 global config value!").arg(changeOrReset(value));
}
