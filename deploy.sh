#!/bin/bash
# deploy.sh — build + bundle into a portable folder
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

# 1. Build
./build.sh

# 2. Prepare output folder
DIST="$ROOT/dist/RETRO-Center"
rm -rf "$DIST"
mkdir -p "$DIST/bin" "$DIST/lib" "$DIST/plugins" "$DIST/qml"

# 3. Copy the binary
cp -a build/retrocenter "$DIST/bin/"

# 4. Bundle with linuxdeploy + Qt plugin (recommended)
# Install once: download linuxdeploy and linuxdeploy-plugin-qt AppImages
# from their GitHub releases, put them on PATH or in the repo.

LINUXDEPLOY="${LINUXDEPLOY:-linuxdeploy}"
QT_PLUGIN="${QT_PLUGIN:-linuxdeploy-plugin-qt}"

if command -v "$LINUXDEPLOY" >/dev/null && command -v "$QT_PLUGIN" >/dev/null; then
    "$LINUXDEPLOY" --appdir "$DIST" \
        --executable "$DIST/bin/retrocenter" \
        --plugin qt \
        --output appimage   # or omit --output appimage if you only want the folder
else
    echo "linuxdeploy not found — falling back to manual bundling"
    # Manual fallback (less complete, but works for basic cases)
    # Copy Qt libs that ldd reports
    ldd build/retrocenter | awk '/=>/ {print $3}' | grep -E 'libQt6|libicu|libGL' | while read -r lib; do
        [ -f "$lib" ] && cp -a "$lib" "$DIST/lib/"
    done

    # Copy essential plugins (adjust paths to your Qt install)
    QT_PLUGINS="${QT_INSTALL_PLUGINS:-$HOME/Qt/6.12.0/gcc_64/plugins}"
    if [ -d "$QT_PLUGINS" ]; then
        cp -a "$QT_PLUGINS/platforms" "$DIST/plugins/"
        cp -a "$QT_PLUGINS/qml"/* "$DIST/qml/" 2>/dev/null || true
        # Add more as needed: imageformats, iconengines, etc.
    fi

    # qt.conf so the binary looks next to itself
    cat > "$DIST/bin/qt.conf" << 'EOF'
[Paths]
Prefix = ..
Libraries = lib
Plugins = plugins
Qml2Imports = qml
EOF
fi

echo "Portable build is in: $DIST"
echo "Try: $DIST/bin/retrocenter"
