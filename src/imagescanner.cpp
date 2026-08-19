#include "imagescanner.h"

#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QSet>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>

#include <algorithm>

namespace {
const QStringList kImageFilters = {
    QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"),
    QStringLiteral("*.png"), QStringLiteral("*.webp"),
    QStringLiteral("*.gif"), QStringLiteral("*.bmp")
};
}

ImageScanner::ImageScanner(QObject *parent)
    : QAbstractListModel(parent)
{
    qRegisterMetaType<ImageEntry>("ImageEntry");
    qRegisterMetaType<QVector<ImageEntry>>("QVector<ImageEntry>");

    connect(&m_watcher, &QFutureWatcher<QVector<ImageEntry>>::finished,
            this, &ImageScanner::onScanFinished);

    refresh();
}

int ImageScanner::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_entries.count();
}

QVariant ImageScanner::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.count())
        return {};

    const ImageEntry &entry = m_entries.at(index.row());
    switch (role) {
    case PathRole:
        return entry.path;
    case FileUrlRole:
        return QUrl::fromLocalFile(entry.path).toString();
    case FileNameRole:
        return entry.fileName;
    case TimestampRole:
        return entry.timestamp;
    case DateSectionRole:
        return entry.dateSection;
    default:
        return {};
    }
}

QHash<int, QByteArray> ImageScanner::roleNames() const
{
    return {
        { PathRole, "path" },
        { FileUrlRole, "fileUrl" },
        { FileNameRole, "fileName" },
        { TimestampRole, "timestamp" },
        { DateSectionRole, "dateSection" }
    };
}

void ImageScanner::refresh()
{
    if (m_scanning)
        return;

    m_scanning = true;
    emit scanningChanged();

    m_watcher.setFuture(QtConcurrent::run(&ImageScanner::scanFolders));
}

void ImageScanner::onScanFinished()
{
    beginResetModel();
    m_entries = m_watcher.result();
    endResetModel();

    m_scanning = false;
    emit scanningChanged();
    emit countChanged();
}

QVector<ImageEntry> ImageScanner::scanFolders()
{
    const QString home = QDir::homePath();
    const QStringList folders = {
        home + QStringLiteral("/Pictures"),
        home + QStringLiteral("/DCIM"),
        home + QStringLiteral("/DCIM/Camera")
    };

    QVector<ImageEntry> entries;
    QSet<QString> seenPaths;

    for (const QString &folderPath : folders) {
        QDir dir(folderPath);
        if (!dir.exists())
            continue;

        const QFileInfoList fileInfos = dir.entryInfoList(kImageFilters, QDir::Files, QDir::Name);
        for (const QFileInfo &info : fileInfos) {
            const QString canonicalPath = info.canonicalFilePath();
            if (canonicalPath.isEmpty() || seenPaths.contains(canonicalPath))
                continue;
            seenPaths.insert(canonicalPath);

            QDateTime created = info.birthTime();
            if (!created.isValid())
                created = info.lastModified();

            ImageEntry entry;
            entry.path = info.absoluteFilePath();
            entry.fileName = info.fileName();
            entry.timestamp = created.toMSecsSinceEpoch();
            entry.dateSection = sectionForTimestamp(entry.timestamp);
            entries.append(entry);
        }
    }

    std::sort(entries.begin(), entries.end(), [](const ImageEntry &a, const ImageEntry &b) {
        return a.timestamp > b.timestamp;
    });

    return entries;
}

QString ImageScanner::sectionForTimestamp(qint64 timestamp)
{
    const QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestamp);
    const QDate date = dt.date();
    const QDate today = QDate::currentDate();

    if (date == today)
        return QStringLiteral("Today");
    if (date == today.addDays(-1))
        return QStringLiteral("Yesterday");

    return date.toString(QStringLiteral("MMMM d, yyyy"));
}
