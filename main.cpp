#include "bot.h"
#include "database.h"
#include "redis.h"
#include "signalhandler.h"
#include "modules/board.h"

#include <telegram.h>

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    SignalHandler signalHandler;

    app.setApplicationName("TelegramBot");
    app.setApplicationVersion("0.1");

    qputenv("QT_LOGGING_RULES", "tg.*=false");
    qputenv("QT_LOGGING_RULES", "tg.*=false\nbot.*.debug=false");

    Database database;
    Redis redis;

    Bot bot(&database, &redis);
    bot.addModule(BOT_MODULE(Board));
    bot.init();

    return app.exec();
}
