#include "imagescanner.h"

#include <QDir>
#include <QDirIterator>
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
	Q_EMIT scanningChanged();

	m_watcher.setFuture(QtConcurrent::run(&ImageScanner::scanFolders));
}

void ImageScanner::onScanFinished()
{
	beginResetModel();
	m_entries = m_watcher.result();

	m_sections.clear();
	QVariantMap currentSection;
	QVariantList currentItems;
	QString currentSectionTitle;

	for (int i = 0; i < m_entries.count(); ++i) {
		const ImageEntry &entry = m_entries.at(i);
		if (entry.dateSection != currentSectionTitle) {
			if (!currentSectionTitle.isEmpty()) {
				currentSection[QStringLiteral("title")] = currentSectionTitle;
				currentSection[QStringLiteral("countText")] = QString::number(currentItems.count()) + QStringLiteral(" Photos");
				currentSection[QStringLiteral("items")] = currentItems;
				m_sections.append(currentSection);
			}
			currentSectionTitle = entry.dateSection;
			currentItems.clear();
		}

		QVariantMap itemMap;
		itemMap[QStringLiteral("path")] = entry.path;
		itemMap[QStringLiteral("fileName")] = entry.fileName;
		itemMap[QStringLiteral("timestamp")] = entry.timestamp;
		itemMap[QStringLiteral("globalIndex")] = i;
		currentItems.append(itemMap);
	}

	if (!currentSectionTitle.isEmpty()) {
		currentSection[QStringLiteral("title")] = currentSectionTitle;
		currentSection[QStringLiteral("countText")] = QString::number(currentItems.count()) + QStringLiteral(" Photos");
		currentSection[QStringLiteral("items")] = currentItems;
		m_sections.append(currentSection);
	}

	endResetModel();

	m_scanning = false;
	Q_EMIT scanningChanged();
	Q_EMIT countChanged();
	Q_EMIT sectionsChanged();
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

		// Recursive search through subdirectories
		QDirIterator it(folderPath, kImageFilters, QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			const QFileInfo info = it.fileInfo();
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
	return dt.date().toString(QStringLiteral("MMMM yyyy"));
}
