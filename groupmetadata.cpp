#include "groupmetadata.h"

GroupMetadata::GroupMetadata()
{

}

GroupMetadata::GroupMetadata(qint64 id, const QString &title, qint64 adminId)
{
    mId = id;
    mTitle = title;
    mAdminId = adminId;
}
