import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    visible: true
    width: 480
    height: 960
    title: "Gallery"

    // NOTE: assumes a plain StackView-style pageStack. If qml-module-cutie
    // exposes a shared PageStack component with a different push() signature,
    // swap this StackView for that and adjust the two push() calls below.
    StackView {
        id: pageStack
        anchors.fill: parent
        initialItem: galleryGridComponent
    }

    Component {
        id: galleryGridComponent
        GalleryGridPage {
            onImageActivated: (index) => {
                pageStack.push(imageViewerComponent, { currentIndex: index })
            }
        }
    }

    Component {
        id: imageViewerComponent
        ImageViewerPage {
            onBackRequested: pageStack.pop()
        }
    }
}
