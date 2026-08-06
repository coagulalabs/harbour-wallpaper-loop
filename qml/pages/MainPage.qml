import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.wallpaperloop 1.0

Page {
    id: page

    property var intervalPresets: [30, 60, 300, 900, 1800, 3600]

    function formatInterval(seconds) {
        if (seconds < 60)
            return qsTr("%1 s").arg(seconds)
        if (seconds % 3600 === 0)
            return qsTr("%1 h").arg(seconds / 3600)
        if (seconds % 60 === 0)
            return qsTr("%1 min").arg(seconds / 60)
        return qsTr("%1 s").arg(seconds)
    }

    PullDownMenu {
        MenuItem {
            text: qsTr("About")
            onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
        }
        MenuItem {
            text: qsTr("Refresh folder")
            enabled: LoopController.hasFolder
            onClicked: LoopController.refreshPlaylist()
        }
        MenuItem {
            text: qsTr("Choose folder")
            onClicked: pageStack.push(Qt.resolvedUrl("FolderPage.qml"))
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: qsTr("Wallpaper Loop") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: LoopController.enabled ? Theme.highlightColor : Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: LoopController.statusText
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: {
                    var folder = LoopController.hasFolder
                                 ? LoopController.folderName
                                 : qsTr("No folder")
                    return folder + " · " + LoopController.positionText
                           + (LoopController.currentName
                              ? " · " + LoopController.currentName
                              : "")
                }
            }

            SectionHeader { text: qsTr("Controls") }

            TextSwitch {
                width: parent.width
                text: qsTr("Slideshow")
                description: qsTr("Cycles images from the chosen folder as your Ambience wallpaper")
                checked: LoopController.enabled
                automaticCheck: false
                onClicked: LoopController.setEnabled(!LoopController.enabled)
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingLarge

                Button {
                    text: qsTr("Previous")
                    enabled: LoopController.canSkip
                    onClicked: LoopController.previous()
                }
                Button {
                    text: qsTr("Next")
                    enabled: LoopController.mediaCount > 0
                    onClicked: LoopController.next()
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: LoopController.hasFolder
                      ? qsTr("Change folder")
                      : qsTr("Choose folder")
                onClicked: pageStack.push(Qt.resolvedUrl("FolderPage.qml"))
            }

            SectionHeader { text: qsTr("Interval") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                color: Theme.highlightColor
                text: formatInterval(LoopController.intervalSeconds)
            }

            Slider {
                id: intervalSlider
                width: parent.width
                minimumValue: 15
                maximumValue: 7200
                stepSize: 15
                value: LoopController.intervalSeconds
                valueText: formatInterval(Math.round(value))
                label: qsTr("Change every")
                onReleased: LoopController.setIntervalSeconds(Math.round(value))
            }

            Flow {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                spacing: Theme.paddingSmall

                Repeater {
                    model: page.intervalPresets
                    Button {
                        preferredWidth: Theme.buttonWidthSmall
                        text: formatInterval(modelData)
                        onClicked: {
                            LoopController.setIntervalSeconds(modelData)
                            intervalSlider.value = modelData
                        }
                    }
                }
            }

            SectionHeader { text: qsTr("Playback") }

            ComboBox {
                width: parent.width
                label: qsTr("Order")
                currentIndex: LoopController.order === "SHUFFLE" ? 1 : 0
                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("In order")
                        onClicked: LoopController.setOrder("SEQUENTIAL")
                    }
                    MenuItem {
                        text: qsTr("Shuffle")
                        onClicked: LoopController.setOrder("SHUFFLE")
                    }
                }
            }

            ComboBox {
                width: parent.width
                label: qsTr("Scaling")
                currentIndex: {
                    if (LoopController.scaleMode === "FIT") return 1
                    if (LoopController.scaleMode === "CONTAIN") return 2
                    return 0
                }
                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Fill — crop to fill")
                        onClicked: LoopController.setScaleMode("FILL")
                    }
                    MenuItem {
                        text: qsTr("Fit — keep aspect ratio")
                        onClicked: LoopController.setScaleMode("FIT")
                    }
                    MenuItem {
                        text: qsTr("Contain — original size, letterbox")
                        onClicked: LoopController.setScaleMode("CONTAIN")
                    }
                }
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Include subfolders")
                checked: LoopController.includeSubfolders
                automaticCheck: false
                onClicked: LoopController.setIncludeSubfolders(!LoopController.includeSubfolders)
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Sailfish uses Ambiences for wallpaper (Settings → Ambiences). "
                           + "This app calls setAmbience for each image — home and lock share it. "
                           + "Supported: JPG, PNG, WebP, BMP, GIF (first frame). "
                           + "Keep the app running or covered for the timer to keep ticking.")
            }
        }
    }
}
