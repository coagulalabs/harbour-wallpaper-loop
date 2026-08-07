import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.wallpaperloop 1.0

CoverBackground {
    Column {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Theme.paddingMedium
        }
        spacing: Theme.paddingSmall

        Label {
            width: parent.width
            text: qsTr("Wallpaper Loop")
            color: Theme.primaryColor
            font.pixelSize: Theme.fontSizeMedium
            truncationMode: TruncationMode.Fade
        }

        Label {
            width: parent.width
            text: {
                if (!LoopController.enabled)
                    return qsTr("Stopped")
                if (LoopController.serviceRunning)
                    return qsTr("Background")
                return qsTr("Starting…")
            }
            color: LoopController.enabled ? Theme.highlightColor : Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
        }

        Label {
            width: parent.width
            visible: LoopController.enabled
            text: qsTr("Controls in Events")
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeExtraSmall
            wrapMode: Text.Wrap
        }

        Label {
            width: parent.width
            text: LoopController.currentName
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            wrapMode: Text.Wrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }
    }

    // No CoverActionList — Next/Previous/Stop live in the Events notification
    // so the home-screen card is not required.
}
