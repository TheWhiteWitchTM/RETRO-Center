import QtQuick
import Theme 1.0

Rectangle {
    id: root
    property string gameId: ""
    property string title: ""
    property string year: ""
    property string developer: ""
    property string shortName: ""
    property string accent: Theme.amber
    property string kind: "classic"
    property bool playable: false
    property bool fileReady: false
    property bool favorite: false
    property bool selected: false
    signal clicked()

    radius: 12
    color: root.selected ? "#2c2a1c" : Theme.card
    border.color: root.selected ? Theme.amber : (hover.containsMouse ? root.accent : Theme.line)
    border.width: root.selected ? 2 : 1

    Rectangle {
        width: 6
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 0
        radius: 3
        color: root.accent
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 18
        anchors.rightMargin: 12
        anchors.topMargin: 12
        anchors.bottomMargin: 10
        spacing: 4

        Row {
            width: parent.width
            spacing: 6
            Text {
                text: root.shortName
                color: root.accent
                font.pixelSize: 11
                font.weight: Font.DemiBold
            }
            Text {
                text: root.favorite ? "★" : ""
                color: Theme.amber
                font.pixelSize: 12
            }
        }
        Text {
            width: parent.width
            text: root.title
            color: Theme.text
            font.pixelSize: 15
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            maximumLineCount: 2
            wrapMode: Text.WordWrap
        }
        Item { width: 1; height: 1; }
        Text {
            width: parent.width
            text: root.playable ? "Ready" : (root.fileReady ? "Image ready" : (root.kind === "classic" ? "Needs your disk" : "Not linked"))
            color: root.playable ? Theme.good : Theme.muted
            font.pixelSize: 12
        }
        Text {
            width: parent.width
            text: (root.year ? root.year + "  " : "") + root.developer
            color: Theme.faint
            font.pixelSize: 11
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: hover
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
