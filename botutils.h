#ifndef BOTUTILS_H
#define BOTUTILS_H

#include <QByteArray>
#include <QVariant>

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
};

#endif // BOTUTILS_H
