import QtQuick
import Theme 1.0

Rectangle {
    id: root
    property string label: ""
    property bool primary: false
    property bool danger: false
    property bool compact: false
    signal clicked()

    implicitHeight: root.compact ? 30 : 36
    implicitWidth: Math.max(root.compact ? 48 : 72, labelText.implicitWidth + (root.compact ? 14 : 28))
    clip: true
    radius: 8
    color: !root.enabled ? Theme.card2 : (mouse.pressed ? Qt.darker(fill, 1.15) : fill)
    border.color: root.primary ? Theme.amber : Theme.line
    border.width: 1
    opacity: root.enabled ? 1 : 0.45

    readonly property color fill: root.primary ? Theme.amber : (root.danger ? "#3a2420" : Theme.card2)

    Text {
        id: labelText
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 6
        anchors.rightMargin: 6
        text: root.label
        color: root.primary ? Theme.ink : (root.danger ? Theme.bad : Theme.text)
        font.pixelSize: root.compact ? 12 : 13
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
