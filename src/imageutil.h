#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

#include <QString>
#include <QStringList>

namespace ImageUtil {

inline QStringList imageFilters()
{
    return QStringList()
           << QStringLiteral("*.jpg") << QStringLiteral("*.jpeg")
           << QStringLiteral("*.png") << QStringLiteral("*.webp")
           << QStringLiteral("*.bmp") << QStringLiteral("*.gif")
           << QStringLiteral("*.JPG") << QStringLiteral("*.JPEG")
           << QStringLiteral("*.PNG") << QStringLiteral("*.WEBP")
           << QStringLiteral("*.BMP") << QStringLiteral("*.GIF");
}

inline bool isImageFile(const QString &fileName)
{
    const QString lower = fileName.toLower();
    return lower.endsWith(QLatin1String(".jpg"))
        || lower.endsWith(QLatin1String(".jpeg"))
        || lower.endsWith(QLatin1String(".png"))
        || lower.endsWith(QLatin1String(".webp"))
        || lower.endsWith(QLatin1String(".bmp"))
        || lower.endsWith(QLatin1String(".gif"));
}

inline int clampIntervalSeconds(int seconds)
{
    const int kMin = 15;
    const int kMax = 86400;
    if (seconds < kMin)
        return kMin;
    if (seconds > kMax)
        return kMax;
    return seconds;
}

inline QString normalizeOrder(const QString &order)
{
    if (order == QLatin1String("SHUFFLE"))
        return QStringLiteral("SHUFFLE");
    return QStringLiteral("SEQUENTIAL");
}

inline QString normalizeScaleMode(const QString &mode)
{
    if (mode == QLatin1String("FIT"))
        return QStringLiteral("FIT");
    if (mode == QLatin1String("CONTAIN"))
        return QStringLiteral("CONTAIN");
    return QStringLiteral("FILL");
}

inline int nextIndex(int current, int count)
{
    if (count <= 0)
        return 0;
    return (current + 1) % count;
}

inline int previousIndex(int current, int count)
{
    if (count <= 0)
        return 0;
    return (current - 1 + count) % count;
}

} // namespace ImageUtil

#endif
