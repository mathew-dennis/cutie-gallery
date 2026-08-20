#pragma once

#include <QAbstractListModel>
#include <QFutureWatcher>
#include <QVector>
#include <QVariantList>
#include <QMetaType>

struct ImageEntry
{
	QString path;
	QString fileName;
	qint64 timestamp = 0;
	QString dateSection;
};

Q_DECLARE_METATYPE(ImageEntry)

class ImageScanner : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
	Q_PROPERTY(int count READ count NOTIFY countChanged)
	Q_PROPERTY(QVariantList sections READ sections NOTIFY sectionsChanged)

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
	QVariantList sections() const { return m_sections; }

	Q_INVOKABLE void refresh();

Q_SIGNALS:
	void scanningChanged();
	void countChanged();
	void sectionsChanged();

private:
	void onScanFinished();

	static QVector<ImageEntry> scanFolders();
	static QString sectionForTimestamp(qint64 timestamp);

	QVector<ImageEntry> m_entries;
	QVariantList m_sections;
	QFutureWatcher<QVector<ImageEntry>> m_watcher;
	bool m_scanning = false;
};
