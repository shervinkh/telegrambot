#ifndef BOTCONFIG_H
#define BOTCONFIG_H

#include "model.h"

class BotInterface;

class BotConfig : private Model
{
private:
    BotInterface *mBotInterface;

    QString mSection;
    QString mName;
    qint64 mVersion;
    QDate mVersionDate;

    Model *mModel;

public:
    BotConfig();
    BotConfig(BotInterface *botInterface, const QString &section, const QString &name,
              qint64 version, const QDate &versionDate);

    QString section() const { return mSection; }
    QString name() const { return mName; }
    QString fullName() const { return QString("%1_%2").arg(mSection).arg(mName); }
    qint64 version() const { return mVersion; }
    QDate versionDate() const { return mVersionDate; }
};

#endif // BOTCONFIG_H
