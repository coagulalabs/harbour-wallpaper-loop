#include "statusnotification.h"
#include "loopcontroller.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QVariantMap>

StatusNotification::StatusNotification(QObject *parent)
    : QObject(parent)
{
}

QString StatusNotification::remoteActionHint(const QString &method)
{
    // Lipstick format: service path interface method [base64-args...]
    // Point at the daemon bus name so lipstick does not launch the Silica UI.
    return QStringLiteral("%1 %2 %3 %4")
            .arg(QLatin1String(LoopController::dbusServiceName()),
                 QLatin1String(LoopController::dbusObjectPath()),
                 QLatin1String(LoopController::dbusInterfaceName()),
                 method);
}

void StatusNotification::show(const QString &summary, const QString &body)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("Notify"));

    QStringList actions;
    actions << QStringLiteral("previous") << QObject::tr("Previous")
            << QStringLiteral("next") << QObject::tr("Next")
            << QStringLiteral("stop") << QObject::tr("Stop");

    QVariantMap hints;
    hints.insert(QStringLiteral("urgency"), 1);
    hints.insert(QStringLiteral("category"), QStringLiteral("x-nemo.system"));
    hints.insert(QStringLiteral("desktop-entry"), QStringLiteral("harbour-wallpaper-loop"));
    hints.insert(QStringLiteral("x-nemo-icon"), QStringLiteral("icon-lock-information"));
    hints.insert(QStringLiteral("x-nemo-feedback"), QStringLiteral(""));
    hints.insert(QStringLiteral("suppress-sound"), true);
    hints.insert(QStringLiteral("resident"), true);
    hints.insert(QStringLiteral("x-nemo-remote-action-previous"),
                 remoteActionHint(QStringLiteral("daemonPrevious")));
    hints.insert(QStringLiteral("x-nemo-remote-action-next"),
                 remoteActionHint(QStringLiteral("daemonNext")));
    hints.insert(QStringLiteral("x-nemo-remote-action-stop"),
                 remoteActionHint(QStringLiteral("stopSlideshow")));

    msg << QStringLiteral("Wallpaper Loop") // app_name
        << m_id                               // replaces_id
        << QStringLiteral("icon-lock-information") // app_icon
        << summary
        << body
        << actions
        << hints
        << 0; // expire_timeout 0 = do not auto-close

    const QDBusReply<uint> reply = QDBusConnection::sessionBus().call(msg);
    if (reply.isValid())
        m_id = reply.value();
}

void StatusNotification::clear()
{
    if (m_id == 0)
        return;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("CloseNotification"));
    msg << m_id;
    QDBusConnection::sessionBus().call(msg);
    m_id = 0;
}
