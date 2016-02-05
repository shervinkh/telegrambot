#include "bot.h"
#include "database.h"
#include "redis.h"
#include "signalhandler.h"
#include "modules/board/board.h"
#include "modules/help/help.h"
#include "modules/subscribe/subscribe.h"
#include "modules/config/configmodule.h"
#include "modules/group/groupmodule.h"

#include <telegram.h>

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    SignalHandler signalHandler;
    Q_UNUSED(signalHandler)

    if (QCoreApplication::arguments().length() < 2)
        qFatal("Specify superuser user-id.");

    bool ok;
    auto headadminId = QCoreApplication::arguments()[1].toLongLong(&ok);

    if (!ok)
        qFatal("Specify superuser user-id.");

    app.setApplicationName("Telegram-Bot");
    app.setApplicationVersion(Bot::version());

    qputenv("QT_LOGGING_RULES", "tg.*=false");
    //qputenv("QT_LOGGING_RULES", "tg.*=false\nbot.*.debug=false");
    qputenv("DEBUG", "true");

    Database database;

    Bot bot(&database, headadminId);
    bot.installModule(new Board);
    bot.installModule(new Help);
    bot.installModule(new Subscribe);
    bot.installModule(new ConfigModule);
    bot.installModule(new GroupModule);
    bot.init();

    return app.exec();
}
