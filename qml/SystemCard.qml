import QtQuick
import QtQuick.Layouts
import Theme 1.0

Rectangle {
    id: root
    property string systemId: ""
    property string shortName: ""
    property string fullName: ""
    property string year: ""
    property string blurb: ""
    property string accent: Theme.amber
    property string secondary: "#ffffff"
    property string art: "generic"
    property bool emulatorReady: false
    property bool biosRequired: false
    property bool biosReady: true
    property int gameCount: 0
    property int readyCount: 0
    signal openLibrary()
    signal powerOn()
    signal setup()

    radius: 14
    color: Theme.card
    border.color: hover.hovered ? root.accent : Theme.line
    border.width: 1
    clip: true

    HoverHandler { id: hover }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 44

            Rectangle { anchors.fill: parent; color: root.accent }

            // Commodore: dark case, bright stripe, a slot.
            Rectangle {
                visible: root.art === "c64"
                anchors.fill: parent
                anchors.margins: 14
                radius: 8
                color: "#241c4a"
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: 10
                    color: "#0c0c0c"
                }
            }

            // Apple II: phosphor lines.
            Column {
                visible: root.art === "apple2"
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4
                Repeater {
                    model: 6
                    Rectangle {
                        width: parent.width * (0.45 + (index % 3) * 0.18)
                        height: 4
                        radius: 2
                        color: "#14300f"
                    }
                }
            }

            // TI: silver bar and a cyan screen.
            Rectangle {
                visible: root.art === "ti99"
                anchors.fill: parent
                color: "#161616"
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 16
                    color: "#c8c8c8"
                }
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 0.55
                    height: 36
                    radius: 4
                    color: root.accent
                }
            }

            // Amiga: a small boing-ball checker.
            Rectangle {
                visible: root.art === "amiga"
                anchors.centerIn: parent
                width: 40
                height: 40
                radius: 20
                color: "#fff6ee"
                clip: true
                Grid {
                    anchors.fill: parent
                    columns: 4
                    rows: 4
                    Repeater {
                        model: 16
                        Rectangle {
                            required property int index
                            width: 10
                            height: 10
                            color: ((Math.floor(index / 4) + index) % 2) ? "#ff8a3d" : "#fff6ee"
                        }
                    }
                }
            }

            // Atari ST: a desktop window.
            Rectangle {
                visible: root.art === "atarist"
                anchors.fill: parent
                anchors.margins: 8
                radius: 4
                color: "#d7d2c4"
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 18
                    color: "#2f6b32"
                }
                Row {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.margins: 10
                    spacing: 8
                    Repeater {
                        model: 3
                        Rectangle { width: 22; height: 16; color: "#2a2a28" }
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 6
                radius: 4
                color: "#cc12130f"
                width: nameText.implicitWidth + 10
                height: nameText.implicitHeight + 2
                Text {
                    id: nameText
                    anchors.centerIn: parent
                    text: root.shortName
                    color: "white"
                    font.pixelSize: 13
                    font.weight: Font.Bold
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 8
            spacing: 3

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: root.fullName
                    color: Theme.text
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Text {
                    text: root.year
                    color: Theme.faint
                    font.pixelSize: 12
                }
            }
            Text {
                Layout.fillWidth: true
                text: !root.emulatorReady ? "Emulator not found"
                     : (root.biosRequired && !root.biosReady ? "Add Kickstart to boot disks" : root.readyCount + " ready to play")
                color: !root.emulatorReady ? Theme.bad : (root.biosRequired && !root.biosReady ? Theme.amber : Theme.good)
                font.pixelSize: 12
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                RButton {
                    label: "Games"
                    compact: true
                    Layout.fillWidth: true
                    Layout.preferredWidth: 64
                    onClicked: root.openLibrary()
                }
                RButton {
                    label: root.emulatorReady ? "Power on" : "Set up"
                    compact: true
                    primary: root.emulatorReady
                    Layout.fillWidth: true
                    Layout.preferredWidth: 96
                    onClicked: root.emulatorReady ? root.powerOn() : root.setup()
                }
            }
        }
    }

    MouseArea {
        id: mouse
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 44
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.openLibrary()
    }
}
