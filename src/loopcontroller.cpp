#include "loopcontroller.h"
#include "imageutil.h"

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
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

#include <algorithm>
#include <random>

namespace {
const char kOrg[] = "harbour-wallpaper-loop";
const char kApp[] = "settings";
const int kDefaultInterval = 300;
}

LoopController::LoopController(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &LoopController::onTick);

    loadSettings();
    rebuildPlaylist(false);

    if (m_enabled && !m_playlist.isEmpty()) {
        applyCurrent();
        scheduleNext();
        setStatusText(tr("Slideshow running"));
    } else if (m_folderPath.isEmpty()) {
        setStatusText(tr("Choose a folder to begin"));
    } else if (m_playlist.isEmpty()) {
        setStatusText(tr("No images in folder"));
    } else {
        setStatusText(tr("Slideshow stopped"));
    }
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
    if (m_playlist.isEmpty() || m_currentIndex < 0 || m_currentIndex >= m_playlist.size())
        return QString();
    return QFileInfo(m_playlist.at(m_currentIndex)).fileName();
}

QString LoopController::currentPath() const
{
    if (m_playlist.isEmpty() || m_currentIndex < 0 || m_currentIndex >= m_playlist.size())
        return QString();
    return m_playlist.at(m_currentIndex);
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

    if (m_enabled) {
        applyCurrent();
        scheduleNext();
        setStatusText(tr("Slideshow running"));
    } else {
        m_timer.stop();
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
        applyCurrent();
        scheduleNext();
        setStatusText(tr("Slideshow running"));
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
    if (m_enabled)
        scheduleNext();
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
        applyCurrent();
        scheduleNext();
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
    if (m_enabled)
        applyCurrent();
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
        applyCurrent();
        scheduleNext();
    } else if (m_playlist.isEmpty()) {
        setStatusText(tr("No images in folder"));
    }
}

void LoopController::next()
{
    if (m_playlist.isEmpty())
        return;
    m_currentIndex = ImageUtil::nextIndex(m_currentIndex, m_playlist.size());
    saveSettings();
    emit playlistChanged();
    applyCurrent();
    if (m_enabled)
        scheduleNext();
}

void LoopController::previous()
{
    if (m_playlist.isEmpty())
        return;
    m_currentIndex = ImageUtil::previousIndex(m_currentIndex, m_playlist.size());
    saveSettings();
    emit playlistChanged();
    applyCurrent();
    if (m_enabled)
        scheduleNext();
}

void LoopController::refreshPlaylist()
{
    rebuildPlaylist(false);
    if (m_enabled && !m_playlist.isEmpty()) {
        applyCurrent();
        scheduleNext();
        setStatusText(tr("Slideshow running"));
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
    if (!m_enabled)
        return;
    next();
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
    if (!m_enabled || m_playlist.isEmpty())
        return;
    m_timer.start(m_intervalSeconds * 1000);
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

    // Sailfish has no Android-style WallpaperManager — home/lock visuals
    // come from Ambiences. setAmbience(file://…) creates/activates one.
    // Prefer the original file (ambienced handles scaling); only stage a
    // local copy when a non-default scale mode is requested.
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
        // contentId 0 = ambienced rejected the URI (seen with bad paths).
        setStatusText(tr("Ambience rejected %1").arg(QFileInfo(sourcePath).fileName()));
        logApply(QStringLiteral("rejected contentId=%1").arg(reply.value()));
        return false;
    }

    logApply(QStringLiteral("ok contentId=%1").arg(reply.value()));
    setStatusText(m_enabled
                  ? tr("Showing %1").arg(QFileInfo(sourcePath).fileName())
                  : tr("Applied %1").arg(QFileInfo(sourcePath).fileName()));
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
    // Ambience wallpapers are often prepared square / high-res; use the
    // larger screen edge so crop/fit looks good in portrait and landscape.
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
    } else { // CONTAIN
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
