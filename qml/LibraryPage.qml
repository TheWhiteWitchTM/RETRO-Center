import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import Theme 1.0

Item {
    id: root
    objectName: "libraryPage"
    property string selectedGameId: ""

    readonly property var current: {
        var revision = library.revision
        if (!root.selectedGameId)
            return null
        var found = library.game(root.selectedGameId)
        return found && found.gameId ? found : null
    }
    onCurrentChanged: {
        if (titleField)
            titleField.text = current ? current.title : ""
    }
    readonly property string heading: {
        var revision = library.revision
        if (!library.systemFilter)
            return "All games"
        var machine = library.system(library.systemFilter)
        return machine && machine.name ? machine.name : "Games"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 16

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        text: root.heading
                        color: Theme.text
                        font.pixelSize: 26
                        font.weight: Font.Bold
                    }
                    Text {
                        text: library.listedCount + " shown"
                        color: Theme.muted
                        font.pixelSize: 12
                    }
                }
                RButton {
                    visible: library.systemFilter !== ""
                    label: "Power on"
                    primary: true
                    onClicked: library.launchSystem(library.systemFilter)
                }
            }

            Row {
                spacing: 8
                Repeater {
                    model: [
                        { key: "all", label: "All" },
                        { key: "ready", label: "Ready" },
                        { key: "missing", label: "Need a disk" },
                        { key: "favorites", label: "Starred" },
                        { key: "recent", label: "Recent" }
                    ]
                    delegate: RButton {
                        required property var modelData
                        label: modelData.label
                        primary: library.listMode === modelData.key
                        onClicked: library.listMode = modelData.key
                    }
                }
            }

            GridView {
                id: grid
                Layout.fillWidth: true
                Layout.fillHeight: true
                cellWidth: 228
                cellHeight: 144
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                model: library.games
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                delegate: Item {
                    id: cell
                    required property string gameId
                    required property string title
                    required property string year
                    required property string developer
                    required property string shortName
                    required property string accent
                    required property string kind
                    required property bool playable
                    required property bool fileReady
                    required property bool favorite
                    width: grid.cellWidth - 12
                    height: grid.cellHeight - 12
                    GameTile {
                        anchors.fill: parent
                        gameId: cell.gameId
                        title: cell.title
                        year: cell.year
                        developer: cell.developer
                        shortName: cell.shortName
                        accent: cell.accent
                        kind: cell.kind
                        playable: cell.playable
                        fileReady: cell.fileReady
                        favorite: cell.favorite
                        selected: root.selectedGameId === cell.gameId
                        onClicked: root.selectedGameId = cell.gameId
                    }
                }
            }
        }

        Rectangle {
            id: detail
            Layout.preferredWidth: 300
            Layout.minimumWidth: 260
            Layout.fillHeight: true
            radius: 16
            color: Theme.card
            border.color: Theme.line
            clip: true
            visible: root.current !== null

            Flickable {
                id: detailScroll
                anchors.fill: parent
                contentWidth: width
                contentHeight: detailColumn.implicitHeight + 24
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            Column {
                id: detailColumn
                x: 14
                y: 8
                width: detailScroll.width - 28
                spacing: 4
                visible: root.current !== null

                Rectangle {
                    width: 54
                    height: 8
                    radius: 4
                    color: root.current ? root.current.accent : Theme.amber
                }
                Text {
                    text: root.current ? root.current.shortName : ""
                    color: root.current ? root.current.accent : Theme.amber
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
                TextField {
                    id: titleField
                    width: parent.width
                    color: Theme.text
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    background: Rectangle { color: "transparent" }
                    onEditingFinished: {
                        if (root.current)
                            library.setTitle(root.current.gameId, text)
                    }
                }
                Text {
                    width: parent.width
                    color: Theme.muted
                    text: root.current ? ((root.current.year ? root.current.year + " · " : "") + root.current.developer) : ""
                }
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    maximumLineCount: 4
                    elide: Text.ElideRight
                    color: Theme.text
                    text: root.current ? root.current.blurb : ""
                }
                Text {
                    width: parent.width
                    wrapMode: Text.WrapAnywhere
                    color: root.current && root.current.playable && root.current.blockReason ? Theme.amber : Theme.muted
                    text: root.current && root.current.blockReason ? root.current.blockReason : (root.current && root.current.path ? root.current.path : "")
                    font.pixelSize: 12
                    font.family: monoFont
                }
                Text {
                    visible: root.current && root.current.lastPlayed
                    color: Theme.faint
                    font.pixelSize: 12
                    text: root.current && root.current.lastPlayed ? "Last played " + root.current.lastPlayed : ""
                }
                RButton {
                    width: detailColumn.width
                    label: "Play"
                    primary: true
                    enabled: root.current && root.current.playable
                    onClicked: library.launchGame(root.current.gameId)
                }
                RButton {
                    width: detailColumn.width
                    label: root.current && root.current.fileReady ? "Change disk image" : "Choose disk image"
                    onClicked: diskDialog.open()
                }
                Row {
                    width: detailColumn.width
                    spacing: 8
                    RButton {
                        width: (parent.width - 8) / 2
                        label: root.current && root.current.favorite ? "Unstar" : "Star"
                        onClicked: library.toggleFavorite(root.current.gameId)
                    }
                    RButton {
                        width: (parent.width - 8) / 2
                        label: "Show file"
                        enabled: root.current && root.current.path
                        onClicked: library.revealPath(root.current.path)
                    }
                }
                RButton {
                    width: detailColumn.width
                    label: "Copy command"
                    onClicked: library.copyText(root.current.command)
                }
                RButton {
                    width: detailColumn.width
                    label: "Remove"
                    danger: true
                    onClicked: removePopup.open()
                }
            }
            }
        }
    }

    FileDialog {
        id: diskDialog
        title: "Choose a disk image"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Images and programs (*.d64 *.g64 *.t64 *.prg *.tap *.crt *.dsk *.do *.po *.nib *.2mg *.ctg *.bin *.rpk *.adf *.adz *.ipf *.dms *.hdf *.st *.msa *.stx *.dim *.tos *.ttp *.img *.rom)", "All files (*)"]
        onAccepted: {
            if (root.current)
                library.setGamePath(root.current.gameId, selectedFile)
        }
    }

    Popup {
        id: removePopup
        modal: true
        anchors.centerIn: parent
        width: 420
        padding: 20
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle { color: Theme.card; radius: 14; border.color: Theme.line }
        Column {
            width: parent.width
            spacing: 14
            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.text
                font.pixelSize: 16
                text: root.current ? ("Remove " + root.current.title + " from the shelf?") : ""
            }
            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.muted
                text: "The file on disk stays where it is. Example cards can be restored from Settings."
            }
            Row {
                spacing: 8
                RButton {
                    label: "Remove"
                    danger: true
                    onClicked: {
                        var id = root.current ? root.current.gameId : ""
                        removePopup.close()
                        root.selectedGameId = ""
                        if (id)
                            library.removeGame(id)
                    }
                }
                RButton { label: "Keep"; onClicked: removePopup.close() }
            }
        }
    }
}
