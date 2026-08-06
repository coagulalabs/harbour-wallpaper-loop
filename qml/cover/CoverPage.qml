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
            text: LoopController.enabled ? qsTr("Running") : qsTr("Stopped")
            color: LoopController.enabled ? Theme.highlightColor : Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
        }

        Label {
            width: parent.width
            text: LoopController.positionText
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
        }

        Label {
            width: parent.width
            text: LoopController.currentName
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeExtraSmall
            wrapMode: Text.Wrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }
    }

    CoverActionList {
        enabled: LoopController.mediaCount > 0

        CoverAction {
            iconSource: "image://theme/icon-cover-previous"
            onTriggered: LoopController.previous()
        }
        CoverAction {
            iconSource: "image://theme/icon-cover-next"
            onTriggered: LoopController.next()
        }
    }
}
