#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>

#include "imagescanner.h"
#include "thumbnailprovider.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	// Plain C++ singleton, exposed the same way NotesManager is in
	// cutie-notes, so QML reaches it as `ImageScanner.count` /
	// `ImageScanner.refresh()` without instantiating anything.
	qmlRegisterSingletonType<ImageScanner>("CutieGallery", 1, 0, "ImageScanner",
		[](QQmlEngine *, QJSEngine *) -> QObject * { return new ImageScanner(); });

	QQmlApplicationEngine engine;
	engine.addImageProvider(QStringLiteral("cutiegallerythumb"), new ThumbnailProvider);

	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(
		&engine, &QQmlApplicationEngine::objectCreated, &app,
		[url](QObject *obj, const QUrl &objUrl) {
			if (!obj && url == objUrl)
				QCoreApplication::exit(-1);
		},
		Qt::QueuedConnection);
	engine.load(url);
	return app.exec();
}
