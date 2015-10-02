#ifndef BOTCONFIG_H
#define BOTCONFIG_H

#include "model.h"
#include "configfield.h"

class BotInterface;
class Redis;

class BotConfig
{
public:
    typedef QMap<QString, ConfigField> ConfigFields;

private:
    static const QString ckGidUidConfigs;

    BotInterface *mBotInterface;

    QString mName;
    qint64 mVersion;
    QDate mVersionDate;

    Model *mModel;
    Redis *mRedis;

    ConfigFields mFields;

    void loadValues();
    void cInvalidateGidUidCache(qint64 gid, qint64 uid = 0);
    bool setGidUidConfig(qint64 gid, qint64 uid, const QString &name, const QVariant &value);\
    bool resetGidUidConfig(qint64 gid, qint64 uid, const QString &name);
    bool canSetConfig(const QString &name, const QVariant &value);

public:
    BotConfig();
    BotConfig(BotInterface *botInterface, const QString &name);

    ConfigField &addField(const QString &fieldName, ConfigField::FieldType fieldType,
                          const QVariant &defaultValue, bool isGlobal = false);

    void registerConfig();
    ConfigFields getConfig(qint64 gid, qint64 uid = 0);

    bool setConfig(qint64 gid, qint64 uid, const QString &name, const QVariant &value);
    bool setDefaultConfig(const QString &name, const QVariant &value);
    bool setGlobalConfig(const QString &name, const QVariant &value);

    QString name() const { return mName; }
};

#endif // BOTCONFIG_H
