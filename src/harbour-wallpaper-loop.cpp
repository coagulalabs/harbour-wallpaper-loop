#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QCoreApplication>
#include <QQmlEngine>
#include <QtQml>
#include <sailfishapp.h>

#include "loopcontroller.h"

static LoopController *g_controller = nullptr;

static QObject *loopControllerProvider(QQmlEngine *engine, QJSEngine *)
{
    Q_UNUSED(engine)
    if (!g_controller)
        g_controller = new LoopController();
    return g_controller;
}

int main(int argc, char *argv[])
{
    // Ensure QSettings / cache paths match Sailjail ApplicationName.
    QCoreApplication::setOrganizationName(QStringLiteral("harbour-wallpaper-loop"));
    QCoreApplication::setApplicationName(QStringLiteral("harbour-wallpaper-loop"));

    qmlRegisterSingletonType<LoopController>(
        "harbour.wallpaperloop", 1, 0, "LoopController", loopControllerProvider);

    return SailfishApp::main(argc, argv);
}
