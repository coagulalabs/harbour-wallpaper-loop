#ifndef STATUSNOTIFICATION_H
#define STATUSNOTIFICATION_H

#include <QObject>
#include <QString>

//! Ongoing Events-view notification with Next / Previous / Stop actions
//! that call the daemon over D-Bus (no UI window required).
class StatusNotification : public QObject
{
    Q_OBJECT
public:
    explicit StatusNotification(QObject *parent = nullptr);

    void show(const QString &summary, const QString &body);
    void clear();

private:
    static QString remoteActionHint(const QString &method);

    uint m_id = 0;
};

#endif
