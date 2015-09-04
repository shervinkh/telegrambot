#ifndef BOTUTILS_H
#define BOTUTILS_H

#include <QByteArray>

class BotUtils
{
public:
    static QByteArray secureRandomBytes(int cnt);
    static quint64 secureRandomLong();
};

#endif // BOTUTILS_H
