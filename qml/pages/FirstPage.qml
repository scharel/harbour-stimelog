import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaListView {
        anchors.fill: parent
        anchors.bottomMargin: panel.visibleSize
        clip: panel.expanded

        header: PageHeader {
            title: qsTr("Time Log")
        }

        PullDownMenu {
            id: pullDownMenu
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("SecondPage.qml"))
            }
        }

        PushUpMenu {
            id: pushUpMenu
            visible: !panel.open
            quickSelect: true
            MenuItem {
                text: qsTr("Add log")
                onClicked: {
                    pushUpMenu.close(true)
                    panel.open = true
                }
            }
        }

        model: 20
        delegate: Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            text: "Item " + index
        }
    }

    DockedPanel {
        id: panel
        width: parent.width
        height: Theme.itemSizeMedium + Theme.paddingLarge
        dock: Dock.Bottom
        open: true

        Row {
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            Label {
                id: timeLabel
                anchors.verticalCenter: parent.verticalCenter
                text: "10:00"
            }

            TextField {
                width: parent.width - timeLabel.width
                anchors.verticalCenter: parent.verticalCenter
                placeholderText: qsTr("project") + ": " + qsTr("task")
                label: qsTr("Add time log")

                EnterKey.enabled: text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: console.log(text)
            }
        }
    }
}
