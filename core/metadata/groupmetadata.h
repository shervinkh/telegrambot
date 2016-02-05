#ifndef GROUPMETADATA_H
#define GROUPMETADATA_H

#include <QObject>

class GroupMetadata
{
private:
    qint64 mId;
    QString mTitle;
    qint64 mAdminId;

public:
    GroupMetadata();
    GroupMetadata(qint64 id, const QString &title, qint64 adminId);

    qint64 id() const { return mId; }
    QString title() const { return mTitle; }
    qint64 adminId() const { return mAdminId; }
};

#endif // GROUPMETADATA_H
