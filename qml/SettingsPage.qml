import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Theme 1.0

Flickable {
    id: root
    contentWidth: width
    contentHeight: column.implicitHeight + 12
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

    Column {
        id: column
        width: root.width
        spacing: 16

        Text {
            text: "Settings"
            color: Theme.text
            font.pixelSize: 28
            font.weight: Font.Bold
        }
        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            color: Theme.muted
            text: "Each machine is one emulator command. Leave the override blank and RETRO Center uses the first program it can find. The shelf itself is a JSON file you can edit."
        }

        Rectangle {
            width: parent.width
            radius: 14
            color: Theme.card
            border.color: Theme.line
            height: shelf.implicitHeight + 28
            Column {
                id: shelf
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 16
                spacing: 8
                Text { text: "Shelf"; color: Theme.text; font.pixelSize: 16; font.weight: Font.DemiBold }
                Text {
                    width: parent.width
                    text: library.dataDir
                    color: Theme.faint
                    font.family: monoFont
                    font.pixelSize: 12
                    wrapMode: Text.WrapAnywhere
                }
                Row {
                    spacing: 8
                    RButton { label: "Open shelf folder"; onClicked: library.openDataFolder() }
                    RButton { label: "Look for emulators"; onClicked: library.probe() }
                    RButton { label: "Restore examples"; onClicked: library.restoreExamples() }
                    RButton { label: "Add a machine"; onClicked: addPopup.open() }
                }
            }
        }

        Rectangle {
            visible: library.busy || library.busyLog.length > 0
            width: parent.width
            radius: 14
            color: Theme.field
            border.color: Theme.line
            height: Math.min(180, logText.implicitHeight + 24)
            clip: true
            Text {
                id: logText
                anchors.fill: parent
                anchors.margins: 12
                text: library.busyLog || "Working…"
                color: Theme.muted
                font.family: monoFont
                font.pixelSize: 11
                wrapMode: Text.Wrap
            }
        }

        Repeater {
            model: library.systems
            delegate: Item {
                id: cell
                required property string systemId
                width: column.width
                height: editor.implicitHeight
                SystemEditor {
                    id: editor
                    width: parent.width
                    systemId: cell.systemId
                }
            }
        }
    }

    Popup {
        id: addPopup
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 460
        padding: 20
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle { color: Theme.card; radius: 14; border.color: Theme.line }

        property string accent: "#e6a23c"

        Column {
            width: parent.width
            spacing: 10
            Text { text: "Add a machine"; color: Theme.text; font.pixelSize: 18; font.weight: Font.DemiBold }
            TextField {
                id: nameField
                width: parent.width
                placeholderText: "Name, for example Amstrad CPC"
                color: Theme.text
                background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
            }
            TextField {
                id: shortField
                width: parent.width
                placeholderText: "Short name"
                color: Theme.text
                background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
            }
            TextField {
                id: extField
                width: parent.width
                placeholderText: "File types, for example .dsk .cdt"
                color: Theme.text
                font.family: monoFont
                background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
            }
            TextField {
                id: commandField
                width: parent.width
                placeholderText: "Command, for example caprice32"
                color: Theme.text
                font.family: monoFont
                background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
            }
            Row {
                spacing: 8
                Repeater {
                    model: ["#e6a23c", "#6f7cff", "#7dff6a", "#3ec7c7", "#ff8a3d", "#d7d2c4"]
                    delegate: Rectangle {
                        required property string modelData
                        width: 28
                        height: 28
                        radius: 14
                        color: modelData
                        border.color: addPopup.accent === modelData ? Theme.text : "transparent"
                        border.width: 2
                        MouseArea { anchors.fill: parent; onClicked: addPopup.accent = modelData }
                    }
                }
            }
            Row {
                spacing: 8
                RButton {
                    label: "Add"
                    primary: true
                    onClicked: {
                        library.addSystem(nameField.text, shortField.text, extField.text, commandField.text, addPopup.accent)
                        nameField.text = ""
                        shortField.text = ""
                        extField.text = ""
                        commandField.text = ""
                        addPopup.close()
                    }
                }
                RButton { label: "Cancel"; onClicked: addPopup.close() }
            }
        }
    }
}
