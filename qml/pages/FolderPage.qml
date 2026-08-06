import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.wallpaperloop 1.0

Page {
    id: page

    property string currentPath: LoopController.hasFolder
                                 ? LoopController.folderPath
                                 : LoopController.picturesPath()

    function reload() {
        folderModel.clear()
        var entries = LoopController.folderEntries(currentPath)
        for (var i = 0; i < entries.length; ++i) {
            folderModel.append({
                name: entries[i],
                path: currentPath + "/" + entries[i]
            })
        }
    }

    Component.onCompleted: reload()

    onCurrentPathChanged: reload()

    SilicaListView {
        id: listView
        anchors {
            fill: parent
            bottomMargin: bottomPanel.height
        }
        model: ListModel { id: folderModel }
        header: PageHeader {
            title: qsTr("Choose folder")
            description: currentPath
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("Go to Pictures")
                onClicked: page.currentPath = LoopController.picturesPath()
            }
            MenuItem {
                text: qsTr("Go to Home")
                onClicked: page.currentPath = LoopController.homePath()
            }
            MenuItem {
                text: qsTr("Use this folder")
                onClicked: {
                    LoopController.setFolderPath(page.currentPath)
                    pageStack.pop()
                }
            }
        }

        delegate: ListItem {
            id: folderItem
            width: listView.width
            contentHeight: Theme.itemSizeMedium

            Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
                text: model.name
                color: folderItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                truncationMode: TruncationMode.Fade
            }

            onClicked: page.currentPath = model.path

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Select")
                    onClicked: {
                        LoopController.setFolderPath(model.path)
                        pageStack.pop()
                    }
                }
            }
        }

        ViewPlaceholder {
            enabled: folderModel.count === 0
            text: qsTr("No subfolders")
            hintText: qsTr("Use the pulley menu to select this folder")
        }

        VerticalScrollDecorator {}
    }

    // Bottom bar: up + select current
    DockedPanel {
        id: bottomPanel
        width: parent.width
        height: Theme.itemSizeLarge
        open: true
        dock: Dock.Bottom

        Row {
            anchors.centerIn: parent
            spacing: Theme.paddingLarge

            Button {
                text: qsTr("Up")
                enabled: page.currentPath !== LoopController.parentFolder(page.currentPath)
                onClicked: page.currentPath = LoopController.parentFolder(page.currentPath)
            }
            Button {
                text: qsTr("Select")
                onClicked: {
                    LoopController.setFolderPath(page.currentPath)
                    pageStack.pop()
                }
            }
        }
    }
}
