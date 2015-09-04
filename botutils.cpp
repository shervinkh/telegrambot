#include "botutils.h"
#include <openssl/rand.h>
#include <QDebug>

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
    QByteArray randomBytes = secureRandomBytes(8);

    quint64 result = 0;
    for (int i = 0; i < 8; i++)
    {
        result <<= 8;
        result |= (quint8) randomBytes[i];
    }

    return result;
}