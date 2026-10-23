# RETRO Center

A small Qt/QML shelf for the machines you already run: Commodore 64, Apple II, TI-99/4A, Amiga, and Atari ST. It powers an emulator on by itself, or starts it with a disk image. Adding another machine is a name, a command, and a list of file types.

Classics on the shelf are cards only. RETRO Center does not include commercial games. Pick a disk image you own, or drop it on the window. A file whose name matches a card fills that card.

Three original demos are included and can be played immediately: Drift on the C64, a boot disk for the Apple II, and a short program for the Atari ST. Cartridges that are already installed for Libre99 are linked when the files are present.

## Build

Qt 6.4 or newer, with Qt Quick and Qt Quick Controls.

```bash
./build.sh
./retrocenter
```

`./retrocenter` runs the build when Qt is installed. `./deploy.sh` builds, then writes `dist/`, the same shape as COSMIC Bar: `dist/retrocenter` launches it, and `dist/bin`, `dist/lib`, `dist/plugins`, and `dist/qml` carry the binary, the Qt libraries, the plugins, and the QML modules. That copy starts on a machine where Qt is not installed.

```bash
./deploy.sh
./dist/retrocenter
```

`build.sh` prefers `~/Qt/6.12.0` and otherwise uses `qmake6`. From a Qt kit you installed yourself:

```bash
QMAKE=/path/to/qt/bin/qmake ./build.sh
```

CMake works the same way if that is what you use:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/Qt/6.12.0/gcc_64
cmake --build build
```

On macOS and Windows, install VICE, LinApple (or another Apple II emulator), a TI-99/4A emulator, FS-UAE, and Hatari, then set each command in Settings if they are not on `PATH`. Power on starts FS-UAE with its built-in AROS ROM. Add a Kickstart you own in Settings to boot original Amiga disks. Hatari runs with its bundled EmuTOS until you add a TOS image.

`./retrocenter --check` prints which emulators were found and whether the bundled demos extracted.

MIT.
