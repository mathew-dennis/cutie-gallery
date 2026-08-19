import QtQuick
import QtQuick.Controls

Page {
    id: page

    signal imageActivated(int index)

    readonly property int smallColumns: 5
    readonly property int largeColumns: 3
    property bool zoomedIn: false
    readonly property int columns: zoomedIn ? largeColumns : smallColumns
    readonly property real cellSize: gridView.width / columns

    header: ToolBar {
        Label {
            anchors.centerIn: parent
            text: "Gallery"
            font.pixelSize: 18
            font.bold: true
        }
    }

    Label {
        anchors.centerIn: parent
        visible: ImageScanner.count === 0 && !ImageScanner.scanning
        text: "No pictures found"
        opacity: 0.6
    }

    BusyIndicator {
        anchors.centerIn: parent
        visible: ImageScanner.scanning && ImageScanner.count === 0
        running: visible
    }

    // Two-stage pinch zoom: pinch out -> fewer, bigger columns,
    // pinch in -> more, smaller columns. Snaps to one of two fixed
    // layouts rather than resizing continuously.
    PinchArea {
        id: pinchArea
        anchors.fill: parent

        onPinchFinished: (pinch) => {
            if (pinch.scale > 1.15 && !page.zoomedIn) {
                page.zoomedIn = true
            } else if (pinch.scale < 0.87 && page.zoomedIn) {
                page.zoomedIn = false
            }
        }

        GridView {
            id: gridView
            anchors.fill: parent
            model: ImageScanner
            cellWidth: page.cellSize
            cellHeight: page.cellSize
            cacheBuffer: cellHeight * 4

            Behavior on cellWidth {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }
            Behavior on cellHeight {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            delegate: Item {
                width: gridView.cellWidth
                height: gridView.cellHeight

                Image {
                    anchors.fill: parent
                    anchors.margins: 1
                    asynchronous: true
                    fillMode: Image.PreserveAspectCrop
                    source: "image://cutiegallerythumb/" + encodeURIComponent(model.path)
                    sourceSize.width: width
                    sourceSize.height: height
                    cache: true

                    Rectangle {
                        anchors.fill: parent
                        color: "#1a1a1a"
                        visible: parent.status !== Image.Ready
                        z: -1
                    }
                }

                TapHandler {
                    onTapped: page.imageActivated(index)
                }
            }
        }
    }
}
