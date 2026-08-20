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

		readonly property int smallColumns: 6
		readonly property int largeColumns: 3
		property bool zoomedIn: false
		readonly property int columns: galleryPage.zoomedIn ? galleryPage.largeColumns : galleryPage.smallColumns
		readonly property real cellSize: sectionListView.width / galleryPage.columns

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
		// in for more, smaller ones. Snaps to two fixed column grid layouts.
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

			ListView {
				id: sectionListView
				anchors.fill: parent
				model: ImageScanner.sections
				spacing: 16
				clip: true
				cacheBuffer: 800

				delegate: Item {
					id: sectionDelegate
					width: sectionListView.width
					property bool expanded: false

					readonly property var sectionItems: modelData.items || []
					readonly property int totalItems: sectionItems.length
					readonly property int defaultMaxCount: galleryPage.columns * 3
					readonly property int visibleCount: sectionDelegate.expanded ? totalItems : Math.min(totalItems, defaultMaxCount)
					readonly property int visibleRows: Math.ceil(visibleCount / galleryPage.columns)

					height: sectionHeader.height + (visibleRows * galleryPage.cellSize) + sectionGrid.anchors.topMargin

					Behavior on height {
						NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
					}

					// Date / Month Section Header Bar (Toggles expanded rows when clicked)
					Item {
						id: sectionHeader
						width: parent.width
						height: 32

						CutieLabel {
							anchors.left: parent.left
							anchors.leftMargin: 8
							anchors.verticalCenter: parent.verticalCenter
							text: modelData.title
							font.pixelSize: 18
							font.bold: true
							color: Atmosphere.textColor
						}

						CutieLabel {
							anchors.right: parent.right
							anchors.rightMargin: 8
							anchors.verticalCenter: parent.verticalCenter
							text: modelData.countText
							font.pixelSize: 13
							color: Atmosphere.textColor
							opacity: 0.7
						}

						TapHandler {
							onTapped: sectionDelegate.expanded = !sectionDelegate.expanded
						}
					}

					// Responsive Grid of Photos
					Grid {
						id: sectionGrid
						anchors.top: sectionHeader.bottom
						anchors.topMargin: 6
						anchors.left: parent.left
						anchors.right: parent.right
						columns: galleryPage.columns

						Repeater {
							model: sectionDelegate.visibleCount

							delegate: Item {
								id: cardRoot
								width: galleryPage.cellSize
								height: galleryPage.cellSize

								readonly property var itemData: sectionDelegate.sectionItems[index]

								Image {
									anchors.fill: parent
									anchors.margins: 1.5
									asynchronous: true
									fillMode: Image.PreserveAspectCrop
									source: "image://cutiegallerythumb/" + encodeURIComponent(cardRoot.itemData.path)
									sourceSize.width: width
									sourceSize.height: height
									cache: true
								}

								// Heart overlay
								CutieLabel {
									anchors.top: parent.top
									anchors.right: parent.right
									anchors.margins: 6
									text: "♥"
									font.pixelSize: 12
									color: "#CCFFFFFF"
								}

								// Video overlay indicator badge
								Item {
									anchors.bottom: parent.bottom
									anchors.right: parent.right
									anchors.margins: 4
									visible: cardRoot.itemData.fileName.endsWith(".mp4") || cardRoot.itemData.fileName.endsWith(".mkv")

									CutieLabel {
										text: "▶ 0:00"
										font.pixelSize: 10
										color: "#FFFFFF"
									}
								}

								TapHandler {
									onTapped: mainWindow.openImage(cardRoot.itemData.globalIndex)
								}
							}
						}
					}
				}
			}
		}

		// Re-scan FAB button
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
