#pragma once

#include <QQuickAsyncImageProvider>

// Decodes downscaled thumbnails off the UI thread, keyed by the
// percent-encoded source file path. No disk cache - just async, on-demand
// decoding at roughly the requested size, kept simple on purpose.
class ThumbnailProvider : public QQuickAsyncImageProvider
{
public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
};
