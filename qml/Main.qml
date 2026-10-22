import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import Theme 1.0

ApplicationWindow {
    id: win
    width: 1100
    height: 700
    minimumWidth: 760
    minimumHeight: 480
    visible: true
    title: "RETRO Center"
    color: Theme.bg

    property string page: "home"
    property int navIndex: 0

    function openLibrary(systemId) {
        library.systemFilter = systemId || ""
        library.listMode = "all"
        page = "library"
        if (libraryPage.selectedGameId) {
            var current = library.game(libraryPage.selectedGameId)
            if (systemId && current && current.systemId !== systemId)
                libraryPage.selectedGameId = ""
        }
    }

    Shortcut {
        sequence: "Ctrl+F"
        onActivated: search.forceActiveFocus()
    }
    Shortcut {
        sequence: "Ctrl+,"
        onActivated: win.page = "settings"
    }
    Shortcut {
        sequence: "Ctrl+Q"
        onActivated: Qt.quit()
    }
    Shortcut {
        sequence: "Return"
        enabled: win.page === "library" && libraryPage.selectedGameId !== ""
                 && !(win.activeFocusItem instanceof TextField)
        onActivated: library.launchGame(libraryPage.selectedGameId)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 232
            Layout.fillHeight: true
            color: Theme.rail

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 4

                Text {
                    text: "RETRO"
                    color: Theme.amber
                    font.pixelSize: 22
                    font.weight: Font.Bold
                    font.letterSpacing: 2
                }
                Text {
                    text: "CENTER"
                    color: Theme.text
                    font.pixelSize: 13
                    font.letterSpacing: 4
                    Layout.bottomMargin: 16
                }

                Repeater {
                    model: [
                        { key: "home", label: "Shelf" },
                        { key: "all", label: "All games" },
                        { key: "favorites", label: "Starred" },
                        { key: "recent", label: "Recent" }
                    ]
                    delegate: Rectangle {
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        radius: 8
                        color: {
                            var on = (modelData.key === "home" && win.page === "home")
                                    || (modelData.key !== "home" && win.page === "library" && library.listMode === (modelData.key === "all" ? "all" : modelData.key) && library.systemFilter === "")
                            return on ? "#242218" : (navMouse.containsMouse ? "#1a1c14" : "transparent")
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            text: modelData.label
                            color: Theme.text
                            font.pixelSize: 14
                        }
                        MouseArea {
                            id: navMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (modelData.key === "home") {
                                    win.page = "home"
                                    library.systemFilter = ""
                                    library.listMode = "all"
                                } else {
                                    library.systemFilter = ""
                                    library.listMode = modelData.key === "all" ? "all" : modelData.key
                                    win.page = "library"
                                }
                            }
                        }
                    }
                }

                Text {
                    text: "MACHINES"
                    color: Theme.faint
                    font.pixelSize: 11
                    font.letterSpacing: 1
                    Layout.topMargin: 16
                    Layout.bottomMargin: 4
                }

                Flickable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentHeight: machineColumn.implicitHeight
                    clip: true
                    Column {
                        id: machineColumn
                        width: parent.width
                        spacing: 2
                        Repeater {
                            model: library.systems
                            delegate: Rectangle {
                                required property string systemId
                                required property string name
                                required property string accent
                                required property bool emulatorReady
                                required property int readyCount
                                width: machineColumn.width
                                height: 34
                                radius: 8
                                color: win.page === "library" && library.systemFilter === systemId ? "#242218" : (machineMouse.containsMouse ? "#1a1c14" : "transparent")
                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 8
                                    spacing: 8
                                    Rectangle {
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: accent
                                    }
                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: parent.width - 48
                                        text: name
                                        color: Theme.text
                                        elide: Text.ElideRight
                                        font.pixelSize: 13
                                    }
                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: emulatorReady ? readyCount : "·"
                                        color: emulatorReady ? Theme.faint : Theme.bad
                                        font.pixelSize: 11
                                    }
                                }
                                MouseArea {
                                    id: machineMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: win.openLibrary(systemId)
                                }
                            }
                        }
                    }
                }

                RButton {
                    Layout.fillWidth: true
                    label: "Settings"
                    primary: win.page === "settings"
                    onClicked: win.page = "settings"
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                color: Theme.bg
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 22
                    anchors.rightMargin: 22
                    spacing: 10
                    TextField {
                        id: search
                        Layout.fillWidth: true
                        Layout.maximumWidth: 520
                        placeholderText: "Search titles, machines, years"
                        color: Theme.text
                        selectByMouse: true
                        leftPadding: 14
                        onTextChanged: library.query = text
                        background: Rectangle {
                            radius: 10
                            color: Theme.field
                            border.color: search.activeFocus ? Theme.amber : Theme.line
                        }
                    }
                    RButton { label: "Add game"; primary: true; onClicked: addPopup.open() }
                    RButton { label: "Scan folder"; onClicked: folderDialog.open() }
                }
            }

            Rectangle {
                visible: library.status !== ""
                Layout.fillWidth: true
                Layout.preferredHeight: statusText.implicitHeight + 14
                Layout.leftMargin: 22
                Layout.rightMargin: 22
                Layout.bottomMargin: 8
                radius: 8
                color: "#221f16"
                Text {
                    id: statusText
                    anchors.fill: parent
                    anchors.margins: 8
                    text: library.status
                    color: Theme.amberSoft
                    wrapMode: Text.WordWrap
                    font.pixelSize: 13
                }
                MouseArea { anchors.fill: parent; onClicked: library.clearStatus() }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 22
                Layout.rightMargin: 22
                Layout.bottomMargin: 16

                HomePage {
                    id: homePage
                    anchors.fill: parent
                    visible: win.page === "home" && search.text === ""
                    onBrowseSystem: function(systemId) { win.openLibrary(systemId) }
                    onShowGame: function(gameId, systemId) {
                        win.openLibrary(systemId)
                        libraryPage.selectedGameId = gameId
                    }
                    onOpenSettings: win.page = "settings"
                }
                LibraryPage {
                    id: libraryPage
                    anchors.fill: parent
                    visible: win.page === "library" || search.text !== ""
                }
                SettingsPage {
                    anchors.fill: parent
                    visible: win.page === "settings" && search.text === ""
                }
            }
        }
    }

    DropArea {
        anchors.fill: parent
        onEntered: function(drag) { dragOverlay.visible = true }
        onExited: dragOverlay.visible = false
        onDropped: function(drop) {
            dragOverlay.visible = false
            if (drop.hasUrls)
                library.addFiles(library.systemFilter, drop.urls)
        }
    }

    Rectangle {
        id: dragOverlay
        anchors.fill: parent
        visible: false
        color: "#cc12130f"
        border.color: Theme.amber
        border.width: 3
        Text {
            anchors.centerIn: parent
            color: Theme.amberSoft
            font.pixelSize: 22
            font.weight: Font.DemiBold
            text: library.systemFilter ? "Drop to add to this machine" : "Drop disk images"
        }
    }

    Popup {
        id: addPopup
        modal: true
        anchors.centerIn: parent
        width: 480
        padding: 20
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle { color: Theme.card; radius: 14; border.color: Theme.line }
        onOpened: {
            var match = 0
            for (var i = 0; i < library.systemCount; ++i) {
                if (library.systemIdAt(i) === library.systemFilter)
                    match = i
            }
            machineBox.currentIndex = match
        }
        Column {
            width: parent.width
            spacing: 12
            Text { text: "Add a game"; color: Theme.text; font.pixelSize: 18; font.weight: Font.DemiBold }
            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.muted
                text: "Pick the machine, then choose one or more files. A file named like a classic fills that card instead of making a second copy."
            }
            ComboBox {
                id: machineBox
                width: parent.width
                model: library.systems
                textRole: "name"
                background: Rectangle { color: Theme.field; radius: 8; border.color: Theme.line }
                contentItem: Text {
                    leftPadding: 12
                    text: machineBox.displayText
                    color: Theme.text
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
            Row {
                spacing: 8
                RButton {
                    label: "Choose files"
                    primary: true
                    onClicked: fileDialog.open()
                }
                RButton { label: "Cancel"; onClicked: addPopup.close() }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Add games"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Images and programs (*.d64 *.g64 *.t64 *.prg *.tap *.crt *.dsk *.do *.po *.nib *.2mg *.ctg *.bin *.rpk *.adf *.adz *.ipf *.dms *.hdf *.st *.msa *.stx *.dim *.tos *.ttp)", "All files (*)"]
        onAccepted: {
            var systemId = library.systemIdAt(machineBox.currentIndex)
            library.addFiles(systemId, selectedFiles)
            addPopup.close()
            win.openLibrary(systemId)
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Scan a folder for games"
        onAccepted: {
            var systemId = win.page === "library" ? library.systemFilter : ""
            library.scanDirectory(systemId, selectedFolder, true)
            if (systemId)
                win.openLibrary(systemId)
            else
                win.page = "library"
        }
    }
}
