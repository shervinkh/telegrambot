#include "bot.h"
#include "database.h"
#include "redis.h"
#include "signalhandler.h"
#include "modules/board/board.h"
#include "modules/help/help.h"
#include "modules/subscribe/subscribe.h"

#include <telegram.h>

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    SignalHandler signalHandler;
    Q_UNUSED(signalHandler)

    app.setApplicationName("Telegram-Bot");
    app.setApplicationVersion(Bot::version());

    qputenv("QT_LOGGING_RULES", "tg.*=false");
    qputenv("QT_LOGGING_RULES", "tg.*=false\nbot.*.debug=false");
    //qputenv("DEBUG", "true");

    Database database;

    Bot bot(&database);
    bot.installModule(MODULE(Board));
    bot.installModule(MODULE(Help));
    bot.installModule(MODULE(Subscribe));
    bot.init();

    return app.exec();
}
