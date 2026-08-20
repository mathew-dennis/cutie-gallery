import Cutie
import CutieGallery
import QtQuick
import QtQuick.Controls

CutiePage {
	id: imageView

    property int currentIndex: 0
    signal backRequested()

    background: Rectangle { color: "black" }

    ListView {
        id: viewerList
        anchors.fill: parent
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
                    active = false
                    if (photo.scale < 1.08) {
                        photo.scale = 1.0
                        photo.x = 0
                        photo.y = 0
                    }
                }
            }

            // Kept as a plain MouseArea (rather than TapHandler) since it
            // sits over the same area as PinchArea - the two are different
            // generations of touch handling in Qt Quick and coexist more
            // predictably than two handlers of the newer pointer-handler kind.
            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    photo.scale = 1.0
                    photo.x = 0
                    photo.y = 0
                }
            }
        }
    }

    ToolButton {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 8
        z: 10
        width: 44
        height: 44

        contentItem: Text {
            text: "\u2190"
            color: "white"
            font.pixelSize: 22
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: width / 2
            color: "#66000000"
        }

        onClicked: mainWindow.pageStack.pop()
    }
}
