#ifndef BOTUTILS_H
#define BOTUTILS_H

#include <QByteArray>
#include <QVariant>
#include <QSqlQuery>

class BotUtils
{
public:
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
};

#endif // BOTUTILS_H
