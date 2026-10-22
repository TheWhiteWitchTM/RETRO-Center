import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Theme 1.0

Flickable {
    id: root
    signal browseSystem(string systemId)
    signal showGame(string gameId, string systemId)
    signal openSettings()

    contentWidth: width
    contentHeight: column.implicitHeight + 28
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

    readonly property var playable: {
        var revision = library.revision
        return library.playableGames()
    }
    readonly property var classics: {
        var revision = library.revision
        return library.classicGames()
    }
    readonly property bool amigaReady: {
        var revision = library.revision
        return library.systemReady("amiga")
    }
    readonly property bool stReady: {
        var revision = library.revision
        return library.systemReady("atarist")
    }


    Column {
        id: column
        width: root.width
        spacing: 22

        Rectangle {
            visible: !root.amigaReady || !root.stReady
            width: parent.width
            height: banner.implicitHeight + 28
            radius: 14
            color: "#2a2418"
            border.color: Theme.amber

            Column {
                id: banner
                anchors.fill: parent
                anchors.margins: 16
                spacing: 10
                Text {
                    width: parent.width
                    text: "Amiga and Atari ST"
                    color: Theme.amberSoft
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    color: Theme.text
                    text: "Hatari and FS-UAE are the emulators for these two. Install them here, or point Settings at a copy you already have."
                }
                Row {
                    spacing: 8
                    RButton {
                        visible: !root.stReady
                        label: library.busy ? "Installing…" : "Install Hatari"
                        primary: true
                        enabled: !library.busy
                        onClicked: library.installFlatpak("atarist")
                    }
                    RButton {
                        visible: !root.amigaReady
                        label: library.busy ? "Installing…" : "Install FS-UAE"
                        primary: true
                        enabled: !library.busy
                        onClicked: library.installFlatpak("amiga")
                    }
                    RButton {
                        label: "Settings"
                        onClicked: root.openSettings()
                    }
                }
            }
        }

        Column {
            width: parent.width
            spacing: 2
            Text {
                text: "The shelf"
                color: Theme.text
                font.pixelSize: 26
                font.weight: Font.Bold
            }
            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.muted
                text: "Power on a machine, or open its games. Drop a disk image anywhere to add it."
            }
        }

        Flow {
            id: machineFlow
            width: parent.width
            spacing: 10
            property int cols: Math.max(1, Math.min(5, Math.floor((Math.max(0, width) + spacing) / 156)))
            Repeater {
                model: library.systems
                delegate: Item {
                    id: cell
                    required property string systemId
                    required property string shortName
                    required property string name
                    required property string year
                    required property string blurb
                    required property string accent
                    required property string secondary
                    required property string art
                    required property bool emulatorReady
                    required property bool biosRequired
                    required property bool biosReady
                    required property int gameCount
                    required property int readyCount
                    width: Math.max(140, Math.floor((machineFlow.width - (machineFlow.cols - 1) * machineFlow.spacing) / machineFlow.cols))
                    height: 132
                    SystemCard {
                        anchors.fill: parent
                        systemId: cell.systemId
                        shortName: cell.shortName
                        fullName: cell.name
                        year: cell.year
                        blurb: cell.blurb
                        accent: cell.accent
                        secondary: cell.secondary
                        art: cell.art
                        emulatorReady: cell.emulatorReady
                        biosRequired: cell.biosRequired
                        biosReady: cell.biosReady
                        gameCount: cell.gameCount
                        readyCount: cell.readyCount
                        onOpenLibrary: root.browseSystem(cell.systemId)
                        onPowerOn: library.launchSystem(cell.systemId)
                        onSetup: root.openSettings()
                    }
                }
            }
        }

        Text {
            text: "Ready to play"
            color: Theme.text
            font.pixelSize: 18
            font.weight: Font.DemiBold
        }
        Text {
            visible: root.playable.length === 0
            width: parent.width
            color: Theme.muted
            wrapMode: Text.WordWrap
            text: "Nothing is ready yet. Power on a machine, or add a disk image."
        }
        Flow {
            width: parent.width
            spacing: 12
            Repeater {
                model: root.playable
                delegate: GameTile {
                    required property var modelData
                    width: 210
                    height: 132
                    gameId: modelData.gameId
                    title: modelData.title
                    year: modelData.year
                    developer: modelData.developer
                    shortName: modelData.shortName
                    accent: modelData.accent
                    kind: modelData.kind
                    playable: true
                    fileReady: true
                    favorite: modelData.favorite
                    onClicked: root.showGame(modelData.gameId, modelData.systemId)
                }
            }
        }

        Text {
            text: "Classics to add"
            color: Theme.text
            font.pixelSize: 18
            font.weight: Font.DemiBold
        }
        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            color: Theme.muted
            text: "These are shelf cards, not copies of the games. Choose a disk image you own and it launches with the right emulator. A matching file name fills the card for you."
        }
        Flow {
            width: parent.width
            spacing: 12
            Repeater {
                model: root.classics
                delegate: GameTile {
                    required property var modelData
                    width: 210
                    height: 132
                    gameId: modelData.gameId
                    title: modelData.title
                    year: modelData.year
                    developer: modelData.developer
                    shortName: modelData.shortName
                    accent: modelData.accent
                    kind: "classic"
                    playable: false
                    fileReady: false
                    onClicked: root.showGame(modelData.gameId, modelData.systemId)
                }
            }
        }
    }
}
