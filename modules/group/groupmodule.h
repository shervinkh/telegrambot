#ifndef GROUPMODULE_H
#define GROUPMODULE_H

#include "module.h"
#include <QPair>

class GroupModule : public Module
{
private:
    static const QString ckUserGroup;

    QString fSet(qint64 uid, qint64 gid);
    QString fUnset(qint64 uid);
    QString fGroupList(qint64 uid);

    qint64 cGetUserGroup(qint64 uid);
    void cInvalidateUserGroup(qint64 uid);

protected:
    ModuleHelp help() const override;
    void registerModels() override;

public:
    GroupModule();
    void init() override;
    void onNewMessage(BInputMessage message);
    QVariant customCommand(const QString &command, const QList<QVariant> &args) override;
};

#endif // GROUPMODULE_H
