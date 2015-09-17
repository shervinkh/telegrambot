#ifndef BOTUTILS_H
#define BOTUTILS_H

#include <QByteArray>
#include <QVariant>
#include <QSqlQuery>

class BotUtils
{
private:
    static const int MAX_RANGE_LEN;

public:
    static const QString DATE_FORMAT;

    static QByteArray secureRandomBytes(int cnt);
    static quint64 secureRandomLong();

    static QList<QVariant> convertArgsToList();

    template<typename... Types>
    static QList<QVariant> convertArgsToList(QVariant value, Types... rest)
    {
        QList<QVariant> result = convertArgsToList(rest...);
        result.prepend(value);
        return result;
    }

    static int getNumRowsAffected(const QSqlQuery &query);
    static QByteArray serialize(const QVariant &input);
    static QVariant deserialize(const QByteArray &input);
    static QList<int> stringToRange(const QString &input, int start = -1, int end = -1);
};

#endif // BOTUTILS_H
