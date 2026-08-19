#include "thumbnailprovider.h"

#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QImageReader>
#include <QThreadPool>
#include <QRunnable>
#include <QUrl>
#include <QImage>

namespace {

constexpr int kDefaultThumbSize = 320;

class ThumbnailResponse : public QQuickImageResponse, public QRunnable
{
public:
    ThumbnailResponse(const QString &path, const QSize &requestedSize)
        : m_path(path)
        , m_requestedSize(requestedSize.isValid() ? requestedSize
                                                    : QSize(kDefaultThumbSize, kDefaultThumbSize))
    {
        // We stay alive after run() so the engine can pull the texture
        // factory from us; QThreadPool must not delete us itself.
        setAutoDelete(false);
    }

    void run() override
    {
        QImageReader reader(m_path);
        reader.setAutoTransform(true);

        const QSize originalSize = reader.size();
        if (originalSize.isValid()) {
            // Expand-to-cover rather than fit, since the grid uses
            // PreserveAspectCrop and we want a sharp crop, not an upscale.
            const QSize scaled = originalSize.scaled(m_requestedSize, Qt::KeepAspectRatioByExpanding);
            reader.setScaledSize(scaled);
        }

        m_image = reader.read();
        if (m_image.isNull())
            m_errorString = reader.errorString();

        emit finished();
    }

    QQuickTextureFactory *textureFactory() const override
    {
        if (m_image.isNull())
            return nullptr;
        return QQuickTextureFactory::textureFactoryForImage(m_image);
    }

    QString errorString() const override { return m_errorString; }

private:
    QString m_path;
    QSize m_requestedSize;
    QImage m_image;
    QString m_errorString;
};

} // namespace

QQuickImageResponse *ThumbnailProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    const QString path = QUrl::fromPercentEncoding(id.toUtf8());
    auto *response = new ThumbnailResponse(path, requestedSize);
    QThreadPool::globalInstance()->start(response);
    return response;
}
