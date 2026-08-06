#ifndef LOOPCONTROLLER_H
#define LOOPCONTROLLER_H

#include <QObject>
#include <QStringList>
#include <QTimer>

class LoopController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString folderPath READ folderPath WRITE setFolderPath NOTIFY folderPathChanged)
    Q_PROPERTY(QString folderName READ folderName NOTIFY folderPathChanged)
    Q_PROPERTY(int intervalSeconds READ intervalSeconds WRITE setIntervalSeconds NOTIFY intervalSecondsChanged)
    Q_PROPERTY(QString order READ order WRITE setOrder NOTIFY orderChanged)
    Q_PROPERTY(QString scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(bool includeSubfolders READ includeSubfolders WRITE setIncludeSubfolders NOTIFY includeSubfoldersChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY playlistChanged)
    Q_PROPERTY(int mediaCount READ mediaCount NOTIFY playlistChanged)
    Q_PROPERTY(QString currentName READ currentName NOTIFY playlistChanged)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY playlistChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString positionText READ positionText NOTIFY playlistChanged)
    Q_PROPERTY(bool hasFolder READ hasFolder NOTIFY folderPathChanged)
    Q_PROPERTY(bool canSkip READ canSkip NOTIFY playlistChanged)

public:
    explicit LoopController(QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    QString folderPath() const { return m_folderPath; }
    QString folderName() const;
    int intervalSeconds() const { return m_intervalSeconds; }
    QString order() const { return m_order; }
    QString scaleMode() const { return m_scaleMode; }
    bool includeSubfolders() const { return m_includeSubfolders; }
    int currentIndex() const { return m_currentIndex; }
    int mediaCount() const { return m_playlist.size(); }
    QString currentName() const;
    QString currentPath() const;
    QString statusText() const { return m_statusText; }
    QString positionText() const;
    bool hasFolder() const { return !m_folderPath.isEmpty(); }
    bool canSkip() const { return m_playlist.size() > 1; }

    Q_INVOKABLE void setEnabled(bool enabled);
    Q_INVOKABLE void setFolderPath(const QString &path);
    Q_INVOKABLE void setIntervalSeconds(int seconds);
    Q_INVOKABLE void setOrder(const QString &order);
    Q_INVOKABLE void setScaleMode(const QString &mode);
    Q_INVOKABLE void setIncludeSubfolders(bool include);
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void refreshPlaylist();
    Q_INVOKABLE void applyCurrent();
    Q_INVOKABLE QStringList folderEntries(const QString &path) const;
    Q_INVOKABLE QString parentFolder(const QString &path) const;
    Q_INVOKABLE QString picturesPath() const;
    Q_INVOKABLE QString homePath() const;

signals:
    void enabledChanged();
    void folderPathChanged();
    void intervalSecondsChanged();
    void orderChanged();
    void scaleModeChanged();
    void includeSubfoldersChanged();
    void playlistChanged();
    void statusTextChanged();

private slots:
    void onTick();

private:
    void loadSettings();
    void saveSettings() const;
    void rebuildPlaylist(bool resetIndex);
    void setStatusText(const QString &text);
    void scheduleNext();
    bool applyPath(const QString &sourcePath);
    QString stageScaledImage(const QString &sourcePath) const;
    QStringList scanImages(const QString &root, bool recursive) const;

    QTimer m_timer;
    bool m_enabled = false;
    bool m_includeSubfolders = true;
    int m_intervalSeconds = 300;
    int m_currentIndex = 0;
    QString m_folderPath;
    QString m_order = QStringLiteral("SEQUENTIAL");
    QString m_scaleMode = QStringLiteral("FILL");
    QString m_statusText;
    QStringList m_playlist;
};

#endif
