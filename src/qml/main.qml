import Cutie
import CutieGallery
import QtQuick
import QtQuick.Controls

CutieWindow {
	id: mainWindow
	width: 480
	height: 960
	visible: true
	title: qsTr("Gallery")

	property var imageViewerComponent: Qt.createComponent("ImageViewerPage.qml")

	function openImage(index) {
		if (mainWindow.imageViewerComponent.status === Component.Ready)
			mainWindow.pageStack.push(mainWindow.imageViewerComponent, { currentIndex: index });
	}

	initialPage: CutiePage {
		id: galleryPage
		width: mainWindow.width
		height: mainWindow.height

		readonly property int smallColumns: 5
		readonly property int largeColumns: 3
		property bool zoomedIn: false
		readonly property int columns: galleryPage.zoomedIn ? galleryPage.largeColumns : galleryPage.smallColumns
		readonly property real cellSize: photoGrid.width / galleryPage.columns

		CutiePageHeader {
			id: header
			title: mainWindow.title
			width: parent.width
		}

		CutieLabel {
			anchors.centerIn: parent
			visible: ImageScanner.count === 0 && !ImageScanner.scanning
			text: qsTr("No pictures yet")
			color: Atmosphere.textColor
			opacity: 0.5
		}

		BusyIndicator {
			anchors.centerIn: parent
			visible: ImageScanner.scanning && ImageScanner.count === 0
			running: visible
		}

		// Two-stage pinch zoom: pinch out for fewer, bigger columns, pinch
		// in for more, smaller ones. Snaps to one of two fixed layouts
		// rather than resizing continuously.
		PinchArea {
			id: pinchArea
			anchors.top: header.bottom
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom

			onPinchFinished: (pinch) => {
				if (pinch.scale > 1.15 && !galleryPage.zoomedIn)
					galleryPage.zoomedIn = true;
				else if (pinch.scale < 0.87 && galleryPage.zoomedIn)
					galleryPage.zoomedIn = false;
			}

			GridView {
				id: photoGrid
				anchors.fill: parent
				model: ImageScanner
				cellWidth: galleryPage.cellSize
				cellHeight: galleryPage.cellSize
				cacheBuffer: cellHeight * 4

				Behavior on cellWidth {
					NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
				}
				Behavior on cellHeight {
					NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
				}

				delegate: Item {
					id: cardRoot
					width: photoGrid.cellWidth
					height: photoGrid.cellHeight

					Image {
						anchors.fill: parent
						anchors.margins: 2
						asynchronous: true
						fillMode: Image.PreserveAspectCrop
						source: "image://cutiegallerythumb/" + encodeURIComponent(model.path)
						sourceSize.width: width
						sourceSize.height: height
						cache: true
					}

					TapHandler {
						onTapped: mainWindow.openImage(index)
					}
				}
			}
		}

		// Re-scan ~/Pictures, ~/DCIM, ~/DCIM/Camera. Same visual language
		// as cutie-notes' new-note FAB: transparent fill, thin ring border.
		Rectangle {
			width: 56
			height: 56
			radius: 28
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.margins: 20
			color: "transparent"
			border.width: 2
			border.color: Atmosphere.textColor
			opacity: ImageScanner.scanning ? 0.4 : 1.0

			CutieLabel {
				anchors.centerIn: parent
				text: "\u21bb"
				font.pixelSize: 24
				color: Atmosphere.textColor
			}

			MouseArea {
				anchors.fill: parent
				enabled: !ImageScanner.scanning
				onClicked: ImageScanner.refresh()
			}
		}
	}
}
