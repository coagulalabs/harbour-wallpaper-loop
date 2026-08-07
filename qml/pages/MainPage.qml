import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.wallpaperloop 1.0

Page {
    id: page

    property var intervalPresets: [30, 60, 300, 900, 1800, 3600, 86400, 604800, 2592000]

    function formatInterval(seconds) {
        if (seconds >= 2592000 && seconds % 2592000 === 0) {
            var months = seconds / 2592000
            return months === 1 ? qsTr("1 month") : qsTr("%1 months").arg(months)
        }
        if (seconds >= 604800 && seconds % 604800 === 0) {
            var weeks = seconds / 604800
            return weeks === 1 ? qsTr("1 week") : qsTr("%1 weeks").arg(weeks)
        }
        if (seconds >= 86400 && seconds % 86400 === 0) {
            var days = seconds / 86400
            return days === 1 ? qsTr("1 day") : qsTr("%1 days").arg(days)
        }
        if (seconds < 60)
            return qsTr("%1 s").arg(seconds)
        if (seconds % 3600 === 0)
            return qsTr("%1 h").arg(seconds / 3600)
        if (seconds % 60 === 0)
            return qsTr("%1 min").arg(seconds / 60)
        return qsTr("%1 s").arg(seconds)
    }

    function syncIntervalSlider() {
        if (LoopController.intervalSeconds <= intervalSlider.maximumValue)
            intervalSlider.value = LoopController.intervalSeconds
    }

    Component.onCompleted: syncIntervalSlider()

    Connections {
        target: LoopController
        onIntervalSecondsChanged: syncIntervalSlider()
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
                description: LoopController.serviceRunning
                             ? qsTr("Running in background — swipe this app away; use Events for Next/Previous/Stop")
                             : qsTr("Starts a background service; then close this app and use Events for controls")
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
                valueText: formatInterval(Math.round(value))
                label: qsTr("Fine adjust (up to 2 h)")
                onReleased: LoopController.setIntervalSeconds(Math.round(value))
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Use presets below for 1 day, 1 week, or 1 month.")
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
                        onClicked: LoopController.setIntervalSeconds(modelData)
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
                           + "After starting the slideshow, close the app: Next / Previous / Stop "
                           + "appear in the Events (notification) view.")
            }
        }
    }
}
