#ifndef METADATA_H
#define METADATA_H

#include "groupmetadata.h"
#include <QObject>

class Redis;

class Metadata : public QObject
{
    Q_OBJECT
private:
    Redis *mMetaRedis;

public:
    Metadata(Redis *metaRedis, QObject *parent);
    GroupMetadata groupMetadata(qint64 gid);
    QList<qint64> userGroups(qint64 uid);
};

#endif // METADATA_H
