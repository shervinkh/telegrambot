#include "groupmetadata.h"

GroupMetadata::GroupMetadata()
    : mId(0), mAdminId(0)
{

}

GroupMetadata::GroupMetadata(qint64 id, const QString &title, qint64 adminId)
{
    mId = id;
    mTitle = title;
    mAdminId = adminId;
}
