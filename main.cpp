#include "bot.h"
#include "database.h"
#include "redis.h"
#include "signalhandler.h"
#include "modules/board/board.h"
#include "modules/help/help.h"

#include <telegram.h>

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    SignalHandler signalHandler;

    app.setApplicationName("Telegram-Bot");
    app.setApplicationVersion(Bot::version());

    qputenv("QT_LOGGING_RULES", "tg.*=false");
    //qputenv("QT_LOGGING_RULES", "tg.*=false\nbot.*.debug=false");
    qputenv("DEBUG", "true");

    Database database;

    Bot bot(&database);
    bot.installModule(MODULE(Board));
    bot.installModule(MODULE(Help));
    bot.init();

    return app.exec();
}
