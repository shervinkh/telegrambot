#include "botconfig.h"
#include "botinterface.h"
#include "redis.h"

#include <QRegularExpression>

const QString BotConfig::ckGidUidConfigs = "gid#%1.uid#%2.configs";

BotConfig::BotConfig()
{

}

BotConfig::BotConfig(BotInterface *botInterface, const QString &name)
    : mBotInterface(botInterface), mName(name)
{
    auto assertWhere = QString("Creating config \"%1\"")
            .arg(name).toLocal8Bit();
    auto assertWhat = QByteArray("Invalid config name. Config name can only contain "
                                 "lowercase english characters and underscore.");
    Q_ASSERT_X(!name.contains(QRegularExpression("[^a-z_]")),
               assertWhere.data(), assertWhat.data());

    mModel = new Model(mBotInterface, "config", mName, 0, QDate(2015, 10, 2));
    mModel->overrideDatabaseTable("config_" + mName);
    mModel->addField("gid", ModelField::Integer);
    mModel->addField("uid", ModelField::Integer);
    mModel->addField("name", ModelField::String);
    mModel->addField("value", ModelField::String);
    mModel->addUniqueIndex("gid", "uid", "name");
    mModel->addIndex("gid", "uid");

    mRedis = new Redis(QString("config.%1").arg(mName));
}

ConfigField &BotConfig::addField(const QString &fieldName, ConfigField::FieldType fieldType,
                                 const QVariant &defaultValue, bool isGlobal)
{
    auto assertWhere = QString("Adding field \"%1\" to config \"%2\"")
            .arg(fieldName).arg(mName).toLocal8Bit();
    auto assertWhatInvalidName = QByteArray("Invalid field name. Field name can only contain "
                                            "lowercase english characters and underscore.");
    auto assertWhatAlreadyExists = QByteArray("Field with the same name already exists.");

    Q_ASSERT_X(!fieldName.contains(QRegularExpression("[^a-z_]")),
               assertWhere.data(), assertWhatInvalidName.data());

    Q_ASSERT_X(!mFields.contains(fieldName), assertWhere.data(),
               assertWhatAlreadyExists.data());

    auto newField = ConfigField(fieldType, isGlobal, defaultValue);
    mFields[fieldName] = newField;
    return mFields[fieldName];
}

void BotConfig::registerConfig()
{
    mBotInterface->registerConfig(this);
    mModel->registerModel();
    loadValues();
}

void BotConfig::loadValues()
{
    auto configs = mModel->objectSet().filter("gid=? AND uid=?", 0, 0).select();

    foreach (auto config, configs)
    {
        auto name = config["name"].toString();
        if (mFields.contains(name))
            mFields[name].mValue = config["value"];
    }
}

BotConfig::ConfigFields BotConfig::getConfig(qint64 gid, qint64 uid)
{
    auto result = mRedis->getCachedValue(ckGidUidConfigs.arg(gid).arg(uid), [this, gid, uid] () -> QVariant {
        QVariantList configs;
        auto modConfigs = mModel->objectSet().filter("gid=? AND uid=?", gid, uid).select();

        foreach (auto modConfig, modConfigs)
        {
            QVariantList thisItem;
            thisItem.append(modConfig["name"]);
            thisItem.append(modConfig["value"]);

            configs.append(QVariant(thisItem));
        }

        return configs;
    });

    ConfigFields configs(mFields);

    auto modConfigs = result.toList();
    foreach (auto modConfig, modConfigs)
    {
        auto elems = modConfig.toList();
        auto name = elems[0].toString();

        if (configs.contains(name) && !configs[name].isGlobal())
            configs[name].mValue = elems[1];
    }

    return configs;
}

void BotConfig::cInvalidateGidUidCache(qint64 gid, qint64 uid)
{
    mRedis->invalidateCache(ckGidUidConfigs.arg(gid).arg(uid));
}

bool BotConfig::canSetConfig(const QString &name, const QVariant &value)
{
    return mFields.contains(name) && (!value.isValid() || mFields[name].canTakeValue(value));
}

bool BotConfig::setGidUidConfig(qint64 gid, qint64 uid, const QString &name, const QVariant &value)
{
    if (!value.isValid())
        return resetGidUidConfig(gid, uid, name);

    auto curValue = mModel->objectSet().filter("gid=? AND uid=? AND name=?", gid, uid, name).select();

    if (!curValue.isEmpty())
    {
        auto config = curValue.first();
        config["value"] = value;
        config->save();
    }
    else
    {
        auto config = mModel->newObject();
        config["gid"] = gid;
        config["uid"] = uid;
        config["name"] = name;
        config["value"] = value;
        config->save();
    }

    if (gid == 0 && uid == 0)
        mFields[name].mValue = value;
    else
        cInvalidateGidUidCache(gid, uid);

    return true;
}

bool BotConfig::resetGidUidConfig(qint64 gid, qint64 uid, const QString &name)
{
    bool res = mModel->objectSet().filter("gid=? AND uid=? AND name=?", gid, uid, name).deleteObjects();

    if (res)
    {
        if (gid == 0 && uid == 0)
            mFields[name].restoreDefault();
        else
            cInvalidateGidUidCache(gid, uid);
    }

    return res;
}

bool BotConfig::setGlobalConfig(const QString &name, const QVariant &value)
{
    if (!canSetConfig(name, value) || !mFields[name].isGlobal())
        return false;

    return setGidUidConfig(0, 0, name, value);
}

bool BotConfig::setDefaultConfig(const QString &name, const QVariant &value)
{
    if (!canSetConfig(name, value) || mFields[name].isGlobal())
        return false;

    return setGidUidConfig(0, 0, name, value);
}

bool BotConfig::setConfig(qint64 gid, qint64 uid, const QString &name, const QVariant &value)
{
    if (!canSetConfig(name, value) || mFields[name].isGlobal())
        return false;

    return setGidUidConfig(gid, uid, name, value);
}
