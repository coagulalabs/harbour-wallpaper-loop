#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QGuiApplication>
#include <QQmlEngine>
#include <QtQml>
#include <sailfishapp.h>

#include "loopcontroller.h"

static LoopController *g_controller = nullptr;

static QObject *loopControllerProvider(QQmlEngine *engine, QJSEngine *)
{
    Q_UNUSED(engine)
    if (!g_controller)
        g_controller = new LoopController(false);
    return g_controller;
}

static int runDaemon(int argc, char *argv[])
{
    // QGuiApplication so FIT/CONTAIN staging can read screen size when available.
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("harbour-wallpaper-loop"));
    QCoreApplication::setApplicationName(QStringLiteral("harbour-wallpaper-loop"));

    LoopController controller(true);
    if (!controller.registerDaemonBus()) {
        qWarning("harbour-wallpaper-loop: failed to claim D-Bus name "
                 "(is another daemon already running?)");
        return 1;
    }

    if (!controller.enabled()) {
        // Leave cleanly; systemd Restart=on-failure will not respawn exit 0.
        qWarning("harbour-wallpaper-loop: slideshow disabled — exiting");
        return 0;
    }

    if (!controller.hasFolder() || controller.mediaCount() <= 0) {
        qWarning("harbour-wallpaper-loop: no images — exiting");
        return 0;
    }

    return app.exec();
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        const QByteArray arg(argv[i]);
        if (arg == "--daemon" || arg == "-d")
            return runDaemon(argc, argv);
    }

    QCoreApplication::setOrganizationName(QStringLiteral("harbour-wallpaper-loop"));
    QCoreApplication::setApplicationName(QStringLiteral("harbour-wallpaper-loop"));

    qmlRegisterSingletonType<LoopController>(
        "harbour.wallpaperloop", 1, 0, "LoopController", loopControllerProvider);

    return SailfishApp::main(argc, argv);
}
