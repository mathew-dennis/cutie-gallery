#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "imagescanner.h"
#include "thumbnailprovider.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("cutie-gallery"));
    app.setOrganizationName(QStringLiteral("cutie-shell-pi"));

    QQmlApplicationEngine engine;

    ImageScanner imageScanner;
    engine.rootContext()->setContextProperty(QStringLiteral("ImageScanner"), &imageScanner);
    engine.addImageProvider(QStringLiteral("cutiegallerythumb"), new ThumbnailProvider);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
