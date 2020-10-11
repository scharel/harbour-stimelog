import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaListView {
        id: listView
        anchors.fill: parent
        anchors.bottomMargin: panel.visibleSize
        clip: panel.expanded

        header: PageHeader {
            title: qsTr("Time Log")
            Timer {
                interval: 100; triggeredOnStart: true; running: page.status === PageStatus.Active; repeat: true;
                onTriggered: {
                    parent.description = Qt.formatTime(new Date(), "hh:mm:ss")
                    var timeDiff = parseInt((new Date() - timeLog.lastTime) / 60 / 1000)
                    timeLabel.text = parseInt(timeDiff / 60) + "h " + timeDiff % 60 + "m"
                }
            }
        }

        PullDownMenu {
            id: pullDownMenu
            MenuItem {
                enabled: false
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            MenuItem {
                enabled: false
                text: qsTr("Edit tasks")
                onClicked: pageStack.push(Qt.resolvedUrl("TasksPage.qml"))
            }
            MenuItem {
                enabled: false
                text: qsTr("Report")
                onClicked: pageStack.push(Qt.resolvedUrl("ReportPage.qml"))
            }
        }

        PushUpMenu {
            id: pushUpMenu
            visible: !panel.open
            quickSelect: true
            MenuItem {
                text: qsTr("Add task")
                onClicked: {
                    pushUpMenu.close(true)
                    panel.open = true
                    taskTextField.focus = true
                }
            }
        }

        model: timeLog
        delegate: ListItem {
            id: listItem
            contentHeight: Theme.iconSizeSmallPlus
            ListView.onRemove: animateRemoval(listItem)

            function remove() {
                remorseAction(qsTr("Deleting"), function() { listView.model.removeRows(index) } )
            }

            Row {
                visible: !commentLabel.visible
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                spacing: Theme.paddingSmall

                Label {
                    id: durationLabel
                    width: Theme.itemSizeMedium
                    color: Theme.secondaryColor
                    text: parseInt(duration / 60) + "h " + duration % 60 + "m"
                }
                Label {
                    id: projectLabel
                    text: project !== "" ? project + ":" : ""
                }
                Label {
                    id: taskLabel
                    width: page.width - x - Theme.horizontalPageMargin
                    truncationMode: TruncationMode.Fade
                    text: task
                }
            }
            Label {
                id: commentLabel
                visible: typeof comment !== 'undefined'
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                color: Theme.secondaryColor
                text: visible ? comment : ""
            }

            onClicked: openMenu()
            menu: ContextMenu {
                MenuLabel {
                    text: "Time: " + [Qt.formatTime(new Date(startTime), "hh:mm"), Qt.formatTime(new Date(endTime), "hh:mm")].filter(function(str) { return str !=="" } ).join(" - ")
                }
                MenuItem {
                    enabled: false
                    text: qsTr("Edit")
                }
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: remove()
                }
            }
        }

            /*Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            text: "Item " + index
        }*/
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
            }

            TextField {
                id: taskTextField
                width: parent.width - timeLabel.width
                anchors.verticalCenter: parent.verticalCenter
                placeholderText: qsTr("project") + ": " + qsTr("task")
                label: qsTr("Add time log")

                //EnterKey.enabled: text.length > 0
                EnterKey.text: qsTr("Add")
                EnterKey.iconSource: text === "" ? "image://theme/icon-m-enter-close" : ""
                EnterKey.onClicked: {
                    if (text !== "") {
                        timeLog.addData(text)
                        text = ""
                    }
                    else {
                        focus = false
                    }
                }
            }
        }
    }
}
