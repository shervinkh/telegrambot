#include "botutils.h"
#include <QSet>
#include <openssl/rand.h>
#include <QDataStream>

const int BotUtils::MAX_RANGE_LEN = 100;
const QString BotUtils::DATE_FORMAT = "yyyy/MM/dd";

QByteArray BotUtils::secureRandomBytes(int cnt)
{
    unsigned char *buffer = new unsigned char[cnt];
    if (!RAND_bytes(buffer, cnt))
        qFatal("Secure random source ended!");
    QByteArray result((const char *) buffer, cnt);
    delete [] buffer;
    return result;
}

quint64 BotUtils::secureRandomLong()
{
    auto randomBytes = secureRandomBytes(8);

    auto result = 0;
    for (int i = 0; i < 8; i++)
    {
        result <<= 8;
        result |= (quint8) randomBytes[i];
    }

    return result;
}

QList<QVariant> BotUtils::convertArgsToList()
{
    return QList<QVariant>();
}

int BotUtils::getNumRowsAffected(const QSqlQuery &query)
{
    auto rawVal = query.numRowsAffected();

    if (rawVal == -1)
        return 0;

    return rawVal;
}

QByteArray BotUtils::serialize(const QVariant &input)
{
    QByteArray result;
    QDataStream DS(&result, QIODevice::Append);
    DS << input;
    return result;
}

QVariant BotUtils::deserialize(const QByteArray &input)
{
    QDataStream DS(input);
    QVariant result;
    DS >> result;
    return result;
}

QList<int> BotUtils::stringToRange(const QString &input, int start, int end)
{
    QSet<int> range;

    auto parts = input.split(',', QString::SkipEmptyParts);

    foreach (auto part, parts)
    {
        auto rangeStrs = part.split('-', QString::SkipEmptyParts);

        if (rangeStrs.size() == 1)
        {
            bool ok;
            auto num = rangeStrs[0].toInt(&ok);
            if (ok)
                range.insert(num);
            else if (rangeStrs[0].toLower().startsWith("all"))
                for (int i = start; i <= end; i++)
                    range.insert(i);
        }
        else if (rangeStrs.size() > 1)
        {
            bool ok1, ok2;

            auto start = rangeStrs[0].toInt(&ok1);
            auto end = rangeStrs[1].toInt(&ok2);

            if (ok1 && ok2)
            {
                if (start < end)
                    end++;
                else if (start > end)
                    end--;

                if (qAbs(end - start) <= MAX_RANGE_LEN)
                    for (int i = start; i != end; (start < end) ? i++ : i--)
                        range.insert(i);
            }
        }
    }

    if (start != -1)
    {
        QMutableSetIterator<int> iter(range);

        while (iter.hasNext())
        {
            auto cur = iter.next();
            if (cur < start || cur > end)
                iter.remove();
        }
    }

    return range.toList();
}
