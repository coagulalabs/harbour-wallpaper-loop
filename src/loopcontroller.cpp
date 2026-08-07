#include "loopcontroller.h"
#include "imageutil.h"
#include "statusnotification.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusReply>

#include <algorithm>
#include <random>

namespace {
const char kOrg[] = "harbour-wallpaper-loop";
const char kApp[] = "settings";
const int kDefaultInterval = 300;
const char kUnit[] = "harbour-wallpaper-loop.service";
}

const char *LoopController::dbusServiceName()
{
    return "harbour.wallpaperloop";
}

const char *LoopController::dbusObjectPath()
{
    return "/harbour/wallpaperloop";
}

const char *LoopController::dbusInterfaceName()
{
    return "harbour.wallpaperloop";
}

const char *LoopController::systemdUnitName()
{
    return kUnit;
}

LoopController::~LoopController()
{
    if (m_statusNotification)
        m_statusNotification->clear();
}

LoopController::LoopController(bool daemonMode, QObject *parent)
    : QObject(parent)
    , m_daemonMode(daemonMode)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &LoopController::onTick);

    loadSettings();
    rebuildPlaylist(false);

    if (m_daemonMode) {
        m_statusNotification = new StatusNotification(this);
        if (m_enabled && !m_playlist.isEmpty()) {
            applyCurrent();
            scheduleNext();
            setStatusText(tr("Background slideshow running"));
            refreshStatusNotification();
        } else if (!m_enabled) {
            setStatusText(tr("Daemon idle — slideshow disabled"));
        } else if (m_folderPath.isEmpty()) {
            setStatusText(tr("Daemon idle — no folder"));
        } else {
            setStatusText(tr("Daemon idle — no images"));
        }
        return;
    }

    // UI mode: never owns the interval timer; poll/start the user service.
    m_servicePoll.setInterval(3000);
    connect(&m_servicePoll, &QTimer::timeout, this, &LoopController::refreshServiceStatus);
    m_servicePoll.start();
    refreshServiceStatus();

    if (m_enabled && !m_folderPath.isEmpty() && !m_playlist.isEmpty()) {
        if (!m_serviceRunning)
            startUserService();
        refreshServiceStatus();
        updateRunningStatusText();
    } else if (m_folderPath.isEmpty()) {
        setStatusText(tr("Choose a folder to begin"));
    } else if (m_playlist.isEmpty()) {
        setStatusText(tr("No images in folder"));
    } else {
        setStatusText(tr("Slideshow stopped"));
    }
}

bool LoopController::registerDaemonBus()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return false;
    if (!bus.registerService(QString::fromLatin1(dbusServiceName())))
        return false;
    if (!bus.registerObject(QString::fromLatin1(dbusObjectPath()), this,
                            QDBusConnection::ExportAllSlots)) {
        bus.unregisterService(QString::fromLatin1(dbusServiceName()));
        return false;
    }
    return true;
}

QString LoopController::folderName() const
{
    if (m_folderPath.isEmpty())
        return QString();
    const QFileInfo info(m_folderPath);
    return info.fileName().isEmpty() ? m_folderPath : info.fileName();
}

QString LoopController::currentName() const
{
    if (!m_playlist.isEmpty() && m_currentIndex >= 0 && m_currentIndex < m_playlist.size())
        return QFileInfo(m_playlist.at(m_currentIndex)).fileName();
    QSettings s(QString::fromLatin1(kOrg), QString::fromLatin1(kApp));
    const QString saved = s.value(QStringLiteral("currentPath")).toString();
    if (!saved.isEmpty())
        return QFileInfo(saved).fileName();
    return QString();
}

QString LoopController::currentPath() const
{
    if (!m_playlist.isEmpty() && m_currentIndex >= 0 && m_currentIndex < m_playlist.size())
        return m_playlist.at(m_currentIndex);
    QSettings s(QString::fromLatin1(kOrg), QString::fromLatin1(kApp));
    return s.value(QStringLiteral("currentPath")).toString();
}

QString LoopController::positionText() const
{
    if (m_playlist.isEmpty())
        return tr("0 of 0");
    return tr("%1 of %2").arg(m_currentIndex + 1).arg(m_playlist.size());
}

void LoopController::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    if (enabled) {
        rebuildPlaylist(false);
        if (m_folderPath.isEmpty()) {
            setStatusText(tr("Choose a folder first"));
            return;
        }
        if (m_playlist.isEmpty()) {
            setStatusText(tr("No images in folder"));
            return;
        }
    }

    m_enabled = enabled;
    saveSettings();
    emit enabledChanged();

    if (m_daemonMode) {
        if (m_enabled) {
            applyCurrent();
            scheduleNext();
            setStatusText(tr("Background slideshow running"));
            refreshStatusNotification();
        } else {
            m_timer.stop();
            m_nextTick = QDateTime();
            if (m_statusNotification)
                m_statusNotification->clear();
            setStatusText(tr("Daemon idle — slideshow disabled"));
        }
        return;
    }

    // UI: control the user service; do not arm a local timer.
    m_timer.stop();
    m_nextTick = QDateTime();
    if (m_enabled) {
        applyCurrent();
        if (!startUserService())
            setStatusText(tr("Could not start background service"));
        else {
            refreshServiceStatus();
            updateRunningStatusText();
        }
    } else {
        stopUserService();
        refreshServiceStatus();
        setStatusText(tr("Slideshow stopped"));
    }
}

void LoopController::setFolderPath(const QString &path)
{
    QString normalized = path;
    if (normalized.startsWith(QStringLiteral("file://")))
        normalized = QUrl(normalized).toLocalFile();

    const QFileInfo info(normalized);
    if (!info.exists() || !info.isDir()) {
        setStatusText(tr("Folder not found"));
        return;
    }

    normalized = info.absoluteFilePath();
    if (m_folderPath == normalized)
        return;

    m_folderPath = normalized;
    m_currentIndex = 0;
    saveSettings();
    emit folderPathChanged();
    rebuildPlaylist(true);

    if (m_enabled && !m_playlist.isEmpty()) {
        if (m_daemonMode) {
            applyCurrent();
            scheduleNext();
            setStatusText(tr("Background slideshow running"));
        } else {
            applyCurrent();
            notifyDaemonReload();
            if (!m_serviceRunning)
                startUserService();
            refreshServiceStatus();
            updateRunningStatusText();
        }
    } else if (m_playlist.isEmpty()) {
        setStatusText(tr("No images in folder"));
    } else {
        setStatusText(tr("Folder ready — start slideshow"));
    }
}

void LoopController::setIntervalSeconds(int seconds)
{
    const int coerced = ImageUtil::clampIntervalSeconds(seconds);
    if (m_intervalSeconds == coerced)
        return;
    m_intervalSeconds = coerced;
    saveSettings();
    emit intervalSecondsChanged();
    maybeSchedule();
    if (!m_daemonMode)
        notifyDaemonReload();
}

void LoopController::setOrder(const QString &order)
{
    const QString next = ImageUtil::normalizeOrder(order);
    if (m_order == next)
        return;
    m_order = next;
    saveSettings();
    emit orderChanged();
    rebuildPlaylist(true);
    if (m_enabled && !m_playlist.isEmpty()) {
        if (m_daemonMode) {
            applyCurrent();
            scheduleNext();
        } else {
            applyCurrent();
            notifyDaemonReload();
        }
    }
}

void LoopController::setScaleMode(const QString &mode)
{
    const QString next = ImageUtil::normalizeScaleMode(mode);

    if (m_scaleMode == next)
        return;
    m_scaleMode = next;
    saveSettings();
    emit scaleModeChanged();
    if (m_enabled) {
        if (m_daemonMode)
            applyCurrent();
        else {
            applyCurrent();
            notifyDaemonReload();
        }
    }
}

void LoopController::setIncludeSubfolders(bool include)
{
    if (m_includeSubfolders == include)
        return;
    m_includeSubfolders = include;
    saveSettings();
    emit includeSubfoldersChanged();
    rebuildPlaylist(true);
    if (m_enabled && !m_playlist.isEmpty()) {
        if (m_daemonMode) {
            applyCurrent();
            scheduleNext();
        } else {
            applyCurrent();
            notifyDaemonReload();
        }
    } else if (m_playlist.isEmpty()) {
        setStatusText(tr("No images in folder"));
    }
}

void LoopController::next()
{
    if (!m_daemonMode && m_enabled && callDaemon(QStringLiteral("daemonNext"))) {
        loadSettings();
        emit playlistChanged();
        emit statusTextChanged();
        refreshServiceStatus();
        updateRunningStatusText();
        return;
    }
    daemonNext();
}

void LoopController::previous()
{
    if (!m_daemonMode && m_enabled && callDaemon(QStringLiteral("daemonPrevious"))) {
        loadSettings();
        emit playlistChanged();
        emit statusTextChanged();
        refreshServiceStatus();
        updateRunningStatusText();
        return;
    }
    daemonPrevious();
}

void LoopController::daemonNext()
{
    if (m_playlist.isEmpty())
        return;
    m_currentIndex = ImageUtil::nextIndex(m_currentIndex, m_playlist.size());
    saveSettings();
    emit playlistChanged();
    applyCurrent();
    maybeSchedule();
}

void LoopController::daemonPrevious()
{
    if (m_playlist.isEmpty())
        return;
    m_currentIndex = ImageUtil::previousIndex(m_currentIndex, m_playlist.size());
    saveSettings();
    emit playlistChanged();
    applyCurrent();
    maybeSchedule();
}

void LoopController::daemonApplyCurrent()
{
    applyCurrent();
}

QString LoopController::ping() const
{
    return QStringLiteral("ok");
}

void LoopController::stopSlideshow()
{
    if (!m_enabled && m_daemonMode) {
        if (m_statusNotification)
            m_statusNotification->clear();
        QCoreApplication::quit();
        return;
    }

    m_enabled = false;
    saveSettings();
    emit enabledChanged();
    m_timer.stop();
    m_nextTick = QDateTime();
    if (m_statusNotification)
        m_statusNotification->clear();

    if (m_daemonMode) {
        setStatusText(tr("Slideshow stopped"));
        // Exit cleanly; Restart=on-failure will not respawn exit 0.
        QCoreApplication::quit();
        return;
    }

    stopUserService();
    refreshServiceStatus();
    setStatusText(tr("Slideshow stopped"));
}

void LoopController::reload()
{
    loadSettings();
    rebuildPlaylist(false);
    if (m_daemonMode) {
        if (m_enabled && !m_playlist.isEmpty()) {
            applyCurrent();
            scheduleNext();
            setStatusText(tr("Background slideshow running"));
            refreshStatusNotification();
        } else {
            m_timer.stop();
            m_nextTick = QDateTime();
            if (m_statusNotification)
                m_statusNotification->clear();
            setStatusText(m_enabled ? tr("Daemon idle — no images")
                                    : tr("Daemon idle — slideshow disabled"));
        }
    } else {
        refreshServiceStatus();
        updateRunningStatusText();
    }
}

void LoopController::refreshPlaylist()
{
    rebuildPlaylist(false);
    if (m_enabled && !m_playlist.isEmpty()) {
        if (m_daemonMode) {
            applyCurrent();
            scheduleNext();
            setStatusText(tr("Background slideshow running"));
        } else {
            applyCurrent();
            notifyDaemonReload();
            updateRunningStatusText();
        }
    }
}

void LoopController::applyCurrent()
{
    if (m_playlist.isEmpty())
        return;
    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size())
        m_currentIndex = 0;
    applyPath(m_playlist.at(m_currentIndex));
}

void LoopController::refreshServiceStatus()
{
    if (m_daemonMode)
        return;
    setServiceRunning(queryServiceActive());
}

QStringList LoopController::folderEntries(const QString &path) const
{
    QString local = path;
    if (local.startsWith(QStringLiteral("file://")))
        local = QUrl(local).toLocalFile();
    if (local.isEmpty())
        local = homePath();

    QDir dir(local);
    if (!dir.exists())
        return QStringList();

    return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
}

QString LoopController::parentFolder(const QString &path) const
{
    QString local = path;
    if (local.startsWith(QStringLiteral("file://")))
        local = QUrl(local).toLocalFile();
    QDir dir(local);
    if (!dir.cdUp())
        return local;
    return dir.absolutePath();
}

QString LoopController::picturesPath() const
{
    const QString pictures = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (!pictures.isEmpty() && QDir(pictures).exists())
        return pictures;
    return homePath() + QStringLiteral("/Pictures");
}

QString LoopController::homePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

void LoopController::onTick()
{
    if (!m_daemonMode || !m_enabled)
        return;
    if (m_nextTick.isValid() && QDateTime::currentDateTimeUtc() < m_nextTick) {
        armTimerChunk();
        return;
    }
    daemonNext();
}

void LoopController::maybeSchedule()
{
    if (m_daemonMode && m_enabled)
        scheduleNext();
}

void LoopController::notifyDaemonReload()
{
    callDaemon(QStringLiteral("reload"));
}

bool LoopController::callDaemon(const QString &method)
{
    QDBusInterface iface(QString::fromLatin1(dbusServiceName()),
                         QString::fromLatin1(dbusObjectPath()),
                         QString::fromLatin1(dbusInterfaceName()),
                         QDBusConnection::sessionBus());
    if (!iface.isValid())
        return false;
    const QDBusMessage reply = iface.call(method);
    return reply.type() != QDBusMessage::ErrorMessage;
}

bool LoopController::queryServiceActive() const
{
    QDBusInterface iface(QString::fromLatin1(dbusServiceName()),
                         QString::fromLatin1(dbusObjectPath()),
                         QString::fromLatin1(dbusInterfaceName()),
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        const QDBusMessage reply = iface.call(QStringLiteral("ping"));
        if (reply.type() != QDBusMessage::ErrorMessage)
            return true;
    }

    QDBusInterface systemd(QStringLiteral("org.freedesktop.systemd1"),
                           QStringLiteral("/org/freedesktop/systemd1"),
                           QStringLiteral("org.freedesktop.systemd1.Manager"),
                           QDBusConnection::sessionBus());
    if (!systemd.isValid())
        return false;

    QDBusMessage getUnit = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.systemd1"),
        QStringLiteral("/org/freedesktop/systemd1"),
        QStringLiteral("org.freedesktop.systemd1.Manager"),
        QStringLiteral("GetUnit"));
    getUnit << QString::fromLatin1(kUnit);
    const QDBusMessage unitReply = QDBusConnection::sessionBus().call(getUnit);
    if (unitReply.type() == QDBusMessage::ErrorMessage)
        return false;

    const QDBusObjectPath unitPath = unitReply.arguments().value(0).value<QDBusObjectPath>();
    QDBusInterface props(QStringLiteral("org.freedesktop.systemd1"),
                         unitPath.path(),
                         QStringLiteral("org.freedesktop.DBus.Properties"),
                         QDBusConnection::sessionBus());
    const QDBusReply<QVariant> active =
            props.call(QStringLiteral("Get"),
                       QStringLiteral("org.freedesktop.systemd1.Unit"),
                       QStringLiteral("ActiveState"));
    return active.isValid() && active.value().toString() == QLatin1String("active");
}

bool LoopController::startUserService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return false;

    QDBusMessage enable = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.systemd1"),
        QStringLiteral("/org/freedesktop/systemd1"),
        QStringLiteral("org.freedesktop.systemd1.Manager"),
        QStringLiteral("EnableUnitFiles"));
    enable << QStringList{QString::fromLatin1(kUnit)} << false << true;
    bus.call(enable);

    QDBusMessage start = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.systemd1"),
        QStringLiteral("/org/freedesktop/systemd1"),
        QStringLiteral("org.freedesktop.systemd1.Manager"),
        QStringLiteral("StartUnit"));
    start << QString::fromLatin1(kUnit) << QStringLiteral("replace");
    const QDBusMessage reply = bus.call(start);
    return reply.type() != QDBusMessage::ErrorMessage;
}

bool LoopController::stopUserService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return false;

    QDBusMessage stop = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.systemd1"),
        QStringLiteral("/org/freedesktop/systemd1"),
        QStringLiteral("org.freedesktop.systemd1.Manager"),
        QStringLiteral("StopUnit"));
    stop << QString::fromLatin1(kUnit) << QStringLiteral("replace");
    bus.call(stop);

    QDBusMessage disable = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.systemd1"),
        QStringLiteral("/org/freedesktop/systemd1"),
        QStringLiteral("org.freedesktop.systemd1.Manager"),
        QStringLiteral("DisableUnitFiles"));
    disable << QStringList{QString::fromLatin1(kUnit)} << false;
    bus.call(disable);
    return true;
}

void LoopController::setServiceRunning(bool running)
{
    if (m_serviceRunning == running)
        return;
    m_serviceRunning = running;
    emit serviceRunningChanged();
}

void LoopController::updateRunningStatusText()
{
    if (!m_enabled) {
        setStatusText(tr("Slideshow stopped"));
        return;
    }
    if (m_serviceRunning)
        setStatusText(tr("Background slideshow running — controls in Events"));
    else
        setStatusText(tr("Slideshow on — starting background service…"));
}

void LoopController::refreshStatusNotification()
{
    if (!m_daemonMode || !m_statusNotification || !m_enabled)
        return;

    const QString name = currentName();
    const QString summary = name.isEmpty() ? tr("Wallpaper Loop") : name;

    QString intervalLabel;
    if (m_intervalSeconds == 86400)
        intervalLabel = tr("1 day");
    else if (m_intervalSeconds == 604800)
        intervalLabel = tr("1 week");
    else if (m_intervalSeconds == 2592000)
        intervalLabel = tr("1 month");
    else if (m_intervalSeconds >= 3600 && m_intervalSeconds % 3600 == 0)
        intervalLabel = tr("%1 h").arg(m_intervalSeconds / 3600);
    else if (m_intervalSeconds >= 60 && m_intervalSeconds % 60 == 0)
        intervalLabel = tr("%1 min").arg(m_intervalSeconds / 60);
    else
        intervalLabel = tr("%1 s").arg(m_intervalSeconds);

    m_statusNotification->show(
        summary,
        tr("%1 · every %2 · Events: Next / Previous / Stop")
            .arg(positionText(), intervalLabel));
}

void LoopController::loadSettings()
{
    QSettings s(QString::fromLatin1(kOrg), QString::fromLatin1(kApp));
    m_folderPath = s.value(QStringLiteral("folderPath")).toString();
    m_intervalSeconds = ImageUtil::clampIntervalSeconds(
            s.value(QStringLiteral("intervalSeconds"), kDefaultInterval).toInt());
    m_enabled = s.value(QStringLiteral("enabled"), false).toBool();
    m_order = ImageUtil::normalizeOrder(
            s.value(QStringLiteral("order"), QStringLiteral("SEQUENTIAL")).toString());
    m_scaleMode = ImageUtil::normalizeScaleMode(
            s.value(QStringLiteral("scaleMode"), QStringLiteral("FILL")).toString());
    m_includeSubfolders = s.value(QStringLiteral("includeSubfolders"), true).toBool();
    m_currentIndex = qMax(0, s.value(QStringLiteral("currentIndex"), 0).toInt());
}

void LoopController::saveSettings() const
{
    QSettings s(QString::fromLatin1(kOrg), QString::fromLatin1(kApp));
    s.setValue(QStringLiteral("folderPath"), m_folderPath);
    s.setValue(QStringLiteral("intervalSeconds"), m_intervalSeconds);
    s.setValue(QStringLiteral("enabled"), m_enabled);
    s.setValue(QStringLiteral("order"), m_order);
    s.setValue(QStringLiteral("scaleMode"), m_scaleMode);
    s.setValue(QStringLiteral("includeSubfolders"), m_includeSubfolders);
    s.setValue(QStringLiteral("currentIndex"), m_currentIndex);
    if (!m_playlist.isEmpty() && m_currentIndex >= 0 && m_currentIndex < m_playlist.size())
        s.setValue(QStringLiteral("currentPath"), m_playlist.at(m_currentIndex));
    s.sync();
}

void LoopController::rebuildPlaylist(bool resetIndex)
{
    m_playlist = scanImages(m_folderPath, m_includeSubfolders);

    if (m_order == QLatin1String("SHUFFLE") && !m_playlist.isEmpty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(m_playlist.begin(), m_playlist.end(), gen);
    } else {
        m_playlist.sort(Qt::CaseInsensitive);
    }

    if (resetIndex || m_currentIndex >= m_playlist.size())
        m_currentIndex = 0;

    saveSettings();
    emit playlistChanged();
}

void LoopController::setStatusText(const QString &text)
{
    if (m_statusText == text)
        return;
    m_statusText = text;
    emit statusTextChanged();
}

void LoopController::scheduleNext()
{
    m_timer.stop();
    m_nextTick = QDateTime();
    if (!m_daemonMode || !m_enabled || m_playlist.isEmpty())
        return;
    m_nextTick = QDateTime::currentDateTimeUtc().addSecs(m_intervalSeconds);
    armTimerChunk();
}

void LoopController::armTimerChunk()
{
    if (!m_daemonMode || !m_enabled || !m_nextTick.isValid())
        return;

    const qint64 remainingMs = QDateTime::currentDateTimeUtc().msecsTo(m_nextTick);
    if (remainingMs <= 0) {
        m_timer.stop();
        QMetaObject::invokeMethod(this, "onTick", Qt::QueuedConnection);
        return;
    }

    static const int kMaxChunkMs = 6 * 60 * 60 * 1000;
    const int chunk = static_cast<int>(qMin(remainingMs, static_cast<qint64>(kMaxChunkMs)));
    m_timer.start(chunk);
}

bool LoopController::applyPath(const QString &sourcePath)
{
    auto logApply = [](const QString &line) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir().mkpath(dir);
        QFile f(dir + QStringLiteral("/apply.log"));
        if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            f.write(line.toUtf8());
            f.write("\n");
        }
    };

    QString pathForAmbience = sourcePath;
    if (m_scaleMode != QLatin1String("FILL")) {
        const QString staged = stageScaledImage(sourcePath);
        if (staged.isEmpty()) {
            setStatusText(tr("Failed to prepare image"));
            logApply(QStringLiteral("stage failed for %1").arg(sourcePath));
            return false;
        }
        pathForAmbience = staged;
    }

    if (!QFileInfo::exists(pathForAmbience)) {
        setStatusText(tr("Image missing: %1").arg(QFileInfo(sourcePath).fileName()));
        logApply(QStringLiteral("missing %1").arg(pathForAmbience));
        return false;
    }

    const QString uri = QString::fromUtf8(
        QUrl::fromLocalFile(pathForAmbience).toEncoded());
    logApply(QStringLiteral("setAmbience %1").arg(uri));

    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("com.jolla.ambienced"),
        QStringLiteral("/com/jolla/ambienced"),
        QStringLiteral("com.jolla.ambienced"),
        QStringLiteral("setAmbience"));
    msg << uri;

    if (!QDBusConnection::sessionBus().isConnected()) {
        setStatusText(tr("No D-Bus session — cannot reach Ambience"));
        logApply(QStringLiteral("no session bus"));
        return false;
    }

    QDBusReply<qlonglong> reply = QDBusConnection::sessionBus().call(msg, QDBus::Block, 8000);
    if (!reply.isValid()) {
        setStatusText(tr("Ambience error: %1").arg(reply.error().message()));
        logApply(QStringLiteral("dbus error: %1").arg(reply.error().message()));
        return false;
    }
    if (reply.value() <= 0) {
        setStatusText(tr("Ambience rejected %1").arg(QFileInfo(sourcePath).fileName()));
        logApply(QStringLiteral("rejected contentId=%1").arg(reply.value()));
        return false;
    }

    logApply(QStringLiteral("ok contentId=%1").arg(reply.value()));
    setStatusText(m_enabled
                  ? tr("Showing %1").arg(QFileInfo(sourcePath).fileName())
                  : tr("Applied %1").arg(QFileInfo(sourcePath).fileName()));
    if (m_daemonMode && m_enabled)
        refreshStatusNotification();
    return true;
}

QString LoopController::stageScaledImage(const QString &sourcePath) const
{
    QImage source(sourcePath);
    if (source.isNull())
        return QString();

    QScreen *screen = QGuiApplication::primaryScreen();
    int targetW = screen ? screen->size().width() : source.width();
    int targetH = screen ? screen->size().height() : source.height();
    const int edge = qMax(targetW, targetH);
    targetW = edge;
    targetH = edge;

    qreal scale = 1.0;
    const qreal fitScale = qMin(qreal(targetW) / source.width(),
                                qreal(targetH) / source.height());
    if (m_scaleMode == QLatin1String("FILL")) {
        scale = qMax(qreal(targetW) / source.width(),
                     qreal(targetH) / source.height());
    } else if (m_scaleMode == QLatin1String("FIT")) {
        scale = fitScale;
    } else {
        scale = qMin(qreal(1), fitScale);
    }

    QImage canvas(targetW, targetH, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::black);

    const int scaledW = qMax(1, int(source.width() * scale));
    const int scaledH = qMax(1, int(source.height() * scale));
    const QImage scaled = source.scaled(scaledW, scaledH, Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation);

    QPainter painter(&canvas);
    painter.drawImage((targetW - scaled.width()) / 2,
                      (targetH - scaled.height()) / 2,
                      scaled);
    painter.end();

    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir);
    const QString outPath = cacheDir + QStringLiteral("/current-wallpaper.jpg");
    if (!canvas.save(outPath, "JPG", 92))
        return QString();
    return outPath;
}

QStringList LoopController::scanImages(const QString &root, bool recursive) const
{
    QStringList result;
    if (root.isEmpty())
        return result;

    QDir dir(root);
    if (!dir.exists())
        return result;

    const QFileInfoList files = dir.entryInfoList(ImageUtil::imageFilters(), QDir::Files, QDir::Name);
    for (const QFileInfo &info : files) {
        if (ImageUtil::isImageFile(info.fileName()))
            result.append(info.absoluteFilePath());
    }

    if (recursive) {
        const QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &sub : dirs)
            result += scanImages(sub.absoluteFilePath(), true);
    }

    return result;
}
