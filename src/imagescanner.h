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

// Scans ~/Pictures, ~/DCIM and ~/DCIM/Camera (non-recursively) and exposes
// the results as a flat, date-sorted list model. Scanning runs on a
// QtConcurrent worker thread so it never blocks the UI.
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

public slots:
    void refresh();

signals:
    void scanningChanged();
    void countChanged();

private slots:
    void onScanFinished();

private:
    static QVector<ImageEntry> scanFolders();
    static QString sectionForTimestamp(qint64 timestamp);

    QVector<ImageEntry> m_entries;
    QFutureWatcher<QVector<ImageEntry>> m_watcher;
    bool m_scanning = false;
};
