import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import Theme 1.0

Rectangle {
    id: root
    property string systemId: ""
    radius: 14
    color: Theme.card
    border.color: Theme.line
    implicitHeight: editor.implicitHeight + 28

    readonly property var machine: {
        var revision = library.revision
        return library.system(root.systemId)
    }

    function loadFields() {
        if (!root.machine || !root.machine.id)
            return
        commandField.text = root.machine.commandOverride || ""
        gameField.text = root.machine.gameArgs || ""
        plainField.text = root.machine.plainArgs || ""
        biosField.text = root.machine.bios || ""
        extField.text = root.machine.extensions || ""
    }

    Component.onCompleted: loadFields()
    onSystemIdChanged: loadFields()

    function save() {
        library.updateSystem(root.systemId, commandField.text, gameField.text, plainField.text, biosField.text, extField.text)
    }

    ColumnLayout {
        id: editor
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 16
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Rectangle { width: 10; height: 10; radius: 5; color: root.machine && root.machine.accent ? root.machine.accent : Theme.amber }
            Text {
                text: root.machine ? root.machine.name : ""
                color: Theme.text
                font.pixelSize: 18
                font.weight: Font.DemiBold
                Layout.fillWidth: true
            }
            Text {
                text: root.machine && root.machine.ready ? "Ready" : "Not found"
                color: root.machine && root.machine.ready ? Theme.good : Theme.bad
                font.pixelSize: 12
                font.weight: Font.DemiBold
            }
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.muted
            text: root.machine ? root.machine.installHint : ""
        }
        Text {
            Layout.fillWidth: true
            color: Theme.faint
            font.family: monoFont
            font.pixelSize: 12
            elide: Text.ElideRight
            text: root.machine && root.machine.detectedCommand ? root.machine.detectedCommand : "Nothing detected on PATH yet."
        }

        Text { text: "Command override"; color: Theme.muted; font.pixelSize: 12 }
        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: commandField
                Layout.fillWidth: true
                placeholderText: "Leave blank to use the detected command"
                color: Theme.text
                font.family: monoFont
                font.pixelSize: 13
                selectByMouse: true
                background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
            }
            RButton { label: "Browse"; onClicked: commandDialog.open() }
            RButton {
                label: "Detect"
                onClicked: {
                    commandField.text = ""
                    root.save()
                }
            }
        }

        Text { text: "Arguments for a game"; color: Theme.muted; font.pixelSize: 12 }
        TextField {
            id: gameField
            Layout.fillWidth: true
            placeholderText: root.machine && root.machine.gameDefault ? root.machine.gameDefault : "{file}"
            color: Theme.text
            font.family: monoFont
            font.pixelSize: 13
            selectByMouse: true
            background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
        }
        Text {
            Layout.fillWidth: true
            color: Theme.faint
            font.pixelSize: 11
            text: "Use {file} for the disk. Leave this blank for the built-in per-type arguments. Example: --autoboot --d1 {file}"
            wrapMode: Text.WordWrap
        }

        Text { text: "Arguments for power on, with no game"; color: Theme.muted; font.pixelSize: 12 }
        TextField {
            id: plainField
            Layout.fillWidth: true
            placeholderText: root.machine && root.machine.plainDefault ? root.machine.plainDefault : "No extra arguments"
            color: Theme.text
            font.family: monoFont
            font.pixelSize: 13
            selectByMouse: true
            background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
        }

        Text {
            visible: root.machine && root.machine.biosFlag
            text: root.machine && root.machine.biosRequired ? "Kickstart / ROM" : "TOS image, optional"
            color: Theme.muted
            font.pixelSize: 12
        }
        RowLayout {
            visible: root.machine && root.machine.biosFlag
            Layout.fillWidth: true
            TextField {
                id: biosField
                Layout.fillWidth: true
                placeholderText: "No ROM selected"
                color: Theme.text
                font.family: monoFont
                font.pixelSize: 13
                selectByMouse: true
                background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
            }
            RButton { label: "Browse"; onClicked: biosDialog.open() }
            RButton { label: "Clear"; onClicked: biosField.text = "" }
        }
        Text {
            visible: root.machine && root.machine.biosNote
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.muted
            font.pixelSize: 12
            text: root.machine ? root.machine.biosNote : ""
        }

        Text { text: "File types"; color: Theme.muted; font.pixelSize: 12 }
        TextField {
            id: extField
            Layout.fillWidth: true
            placeholderText: ".d64 .prg"
            color: Theme.text
            font.family: monoFont
            font.pixelSize: 13
            selectByMouse: true
            background: Rectangle { color: Theme.field; radius: 8; border.color: parent.activeFocus ? Theme.amber : Theme.line }
        }

        Row {
            spacing: 8
            RButton { label: "Save"; primary: true; onClicked: root.save() }
            RButton { label: "Power on"; enabled: root.machine && root.machine.ready; onClicked: library.launchSystem(root.systemId) }
            RButton {
                visible: root.machine && root.machine.flatpakId && !root.machine.ready
                label: "Install"
                enabled: !library.busy
                onClicked: library.installFlatpak(root.systemId)
            }
            RButton {
                visible: root.machine && !root.machine.builtin
                label: "Remove machine"
                danger: true
                onClicked: library.removeSystem(root.systemId)
            }
        }
    }

    FileDialog {
        id: commandDialog
        title: "Choose the emulator"
        fileMode: FileDialog.OpenFile
        nameFilters: ["All files (*)"]
        onAccepted: commandField.text = library.localPath(selectedFile)
    }
    FileDialog {
        id: biosDialog
        title: "Choose a ROM image"
        fileMode: FileDialog.OpenFile
        nameFilters: ["ROM images (*.rom *.img *.bin)", "All files (*)"]
        onAccepted: biosField.text = library.localPath(selectedFile)
    }
}
