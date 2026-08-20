import Cutie
import CutieGallery
import QtQuick
import QtQuick.Controls

CutiePage {
	id: imageView

	// Set by whoever pushes this page (see main.qml's openImage()).
	property int currentIndex: 0

	CutiePageHeader {
		id: header
		title: qsTr("Photo")
		width: parent.width

		// NoteView.qml doesn't add its own back button, so CutiePageHeader
		// may already provide one automatically once pushed onto
		// pageStack - I don't have visibility into whether it does. This
		// is here explicitly so the back arrow is guaranteed to work;
		// drop it if it turns out to duplicate the header's own.
		CutieButton {
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			anchors.leftMargin: 15
			icon.name: "go-previous-symbolic"
			background: null
			onClicked: mainWindow.pageStack.pop()
		}
	}

	ListView {
		id: viewerList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		model: ImageScanner
		orientation: ListView.Horizontal
		snapMode: ListView.SnapOneItem
		highlightRangeMode: ListView.StrictlyEnforceRange
		highlightMoveDuration: 200
		boundsBehavior: Flickable.StopAtBounds
		cacheBuffer: width
		// Set once from the tapped grid index, then user swipes take over -
		// same idea as NoteView's noteId being set once on push.
		currentIndex: imageView.currentIndex
		// Swiping between photos is disabled while the current photo is
		// pinch-zoomed, so panning doesn't fight paging.
		interactive: !(currentItem && currentItem.zoomed)

		delegate: Item {
			id: photoPage
			width: viewerList.width
			height: viewerList.height
			clip: true

			property bool zoomed: photo.scale > 1.01

			Image {
				id: photo
				anchors.fill: parent
				fillMode: Image.PreserveAspectFit
				asynchronous: true
				autoTransform: true
				cache: false
				source: model.fileUrl

				Behavior on scale {
					enabled: !pinchArea.active
					NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
				}
				Behavior on x {
					enabled: !pinchArea.active
					NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
				}
				Behavior on y {
					enabled: !pinchArea.active
					NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
				}

				BusyIndicator {
					anchors.centerIn: parent
					running: photo.status === Image.Loading
					visible: running
				}
			}

			PinchArea {
				id: pinchArea
				anchors.fill: parent
				property bool active: false

				pinch.target: photo
				pinch.minimumScale: 1.0
				pinch.maximumScale: 4.0
				pinch.minimumX: -photo.width * 1.5
				pinch.maximumX: photo.width * 1.5
				pinch.minimumY: -photo.height * 1.5
				pinch.maximumY: photo.height * 1.5

				onPinchStarted: active = true
				onPinchFinished: {
					active = false;
					if (photo.scale < 1.08) {
						photo.scale = 1.0;
						photo.x = 0;
						photo.y = 0;
					}
				}
			}

			// Plain MouseArea rather than TapHandler for the double-tap -
			// it sits over the same area as PinchArea, and the two are
			// different generations of touch handling in Qt Quick that
			// coexist more predictably than two newer-style handlers would.
			MouseArea {
				anchors.fill: parent
				onDoubleClicked: {
					photo.scale = 1.0;
					photo.x = 0;
					photo.y = 0;
				}
			}
		}
	}
}
