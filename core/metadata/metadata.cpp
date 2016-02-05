#include "metadata.h"
#include "redis.h"

Metadata::Metadata(Redis *metaRedis, QObject *parent)
    : QObject(parent), mMetaRedis(metaRedis)
{
}

GroupMetadata Metadata::groupMetadata(qint64 gid)
{
    if (!mMetaRedis->exists(QString("chat#%1").arg(gid)).toBool())
        return GroupMetadata();

    auto title = mMetaRedis->hget(QString("chat#%1").arg(gid), "title").toString();
    auto adminId = mMetaRedis->hget(QString("chat#%1").arg(gid), "admin").toLongLong();

    return GroupMetadata(gid, title, adminId);
}

QList<qint64> Metadata::userGroups(qint64 uid)
{
    QList<qint64> res;
    auto lst = mMetaRedis->smembers(QString("user#%1.groups").arg(uid)).toList();

    for (auto group : lst)
        res.append(group.toLongLong());

    return res;
}
