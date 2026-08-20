#pragma once

#include <QAbstractListModel>
#include <QFutureWatcher>
#include <QVector>
#include <QMetaType>

struct ImageEntry
{
	QString path;
	QString fileName;
	qint64 timestamp = 0;
	QString dateSection;
};

Q_DECLARE_METATYPE(ImageEntry)

// Backs the photo grid and the swipe viewer. Scans ~/Pictures, ~/DCIM and
// ~/DCIM/Camera (non-recursively, deduped by canonical path) on a
// QtConcurrent worker thread so it never blocks the UI - same idea as
// DriveManager scanning /media in cutie-explorer.
//
// Exposed as a real QAbstractListModel rather than a QVariantList (unlike
// NotesManager.notes) because a photo library can run into the hundreds -
// GridView/ListView can then virtualize it properly instead of the app
// rebuilding one big list on every change.
class ImageScanner : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
	Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
	enum Roles {
		PathRole = Qt::UserRole + 1,
		FileUrlRole,
		FileNameRole,
		TimestampRole,
		DateSectionRole
	};
	Q_ENUM(Roles)

	explicit ImageScanner(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	bool scanning() const { return m_scanning; }
	int count() const { return m_entries.count(); }

	// Re-scans the three folders. Called automatically on startup; exposed
	// to QML too for the refresh FAB, mirroring DriveManager.refresh().
	Q_INVOKABLE void refresh();

Q_SIGNALS:
	void scanningChanged();
	void countChanged();

private:
	void onScanFinished();

	static QVector<ImageEntry> scanFolders();
	static QString sectionForTimestamp(qint64 timestamp);

	QVector<ImageEntry> m_entries;
	QFutureWatcher<QVector<ImageEntry>> m_watcher;
	bool m_scanning = false;
};
