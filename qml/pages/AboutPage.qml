import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    readonly property string appVersion: "1.3.0"

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: qsTr("About") }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "image://theme/icon-l-image"
                sourceSize.width: Theme.iconSizeLarge
                sourceSize.height: Theme.iconSizeLarge
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeLarge
                text: qsTr("Wallpaper Loop")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Version %1").arg(page.appVersion)
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.primaryColor
                text: qsTr("Cycle through a folder of images and apply each one as your "
                           + "Sailfish Ambience wallpaper on a timer.")
            }

            SectionHeader { text: qsTr("Features") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: "• " + qsTr("Folder browser (Pictures, Downloads, Home)")
                      + "\n• " + qsTr("Interval slider and presets (30s–1h)")
                      + "\n• " + qsTr("Sequential or shuffle order")
                      + "\n• " + qsTr("Fill / Fit / Contain scaling")
                      + "\n• " + qsTr("Include subfolders")
                      + "\n• " + qsTr("Next / Previous in app and cover")
                      + "\n• " + qsTr("Settings persist across launches")
            }

            SectionHeader { text: qsTr("Notes") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Sailfish uses Ambiences for wallpaper — home and lock share "
                           + "the same image. Supported stills: JPG, PNG, WebP, BMP, GIF "
                           + "(first frame). Keep the app running or covered for the timer "
                           + "to keep ticking. Animated live wallpaper is not available on Sailfish.")
            }

            SectionHeader { text: qsTr("Links") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                textFormat: Text.PlainText
                text: "OpenRepos: openrepos.net\n"
                      + "Source: github.com/coagulalabs/harbour-wallpaper-loop\n"
                      + "Android original: dev.wallpaper.loop"
            }

            SectionHeader { text: qsTr("License") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("BSD-3-Clause") + "\n"
                      + qsTr("Copyright © 2026 Coagula / coagulalabs")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Package id: harbour-wallpaper-loop")
            }
        }
    }
}
