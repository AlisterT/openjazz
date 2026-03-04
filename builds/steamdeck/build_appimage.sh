#!/usr/bin/env bash
# build_appimage.sh — Build OpenJazz AppImage for Steam Deck (Linux x86_64)
#
# Usage:
#   ./builds/steamdeck/build_appimage.sh
#
# Output: openjazz.appimage
#
# Requires: cmake, ninja, pkg-config, SDL2 dev headers
# Downloads linuxdeploy and appimagetool automatically if absent.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# ── Config ─────────────────────────────────────────────────────────────────
PRESET="steamdeck-release"
BUILD_DIR="$REPO_ROOT/build-$PRESET"
APPDIR="$BUILD_DIR/OpenJazz.AppDir"
TOOLS_DIR="$BUILD_DIR/tools"

LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
APPIMAGETOOL_URL="https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage"

# ── Helpers ─────────────────────────────────────────────────────────────────
info()  { printf '\033[1;32m==> %s\033[0m\n' "$*"; }
error() { printf '\033[1;31mERROR: %s\033[0m\n' "$*" >&2; exit 1; }

need_cmd() {
    command -v "$1" &>/dev/null || error "'$1' not found. Please install it."
}

download_tool() {
    local url="$1"
    local dest="$2"
    if [[ ! -x "$dest" ]]; then
        info "Downloading $(basename "$dest")..."
        mkdir -p "$(dirname "$dest")"
        if command -v curl &>/dev/null; then
            curl -fsSL "$url" -o "$dest"
        elif command -v wget &>/dev/null; then
            wget -q "$url" -O "$dest"
        else
            error "Neither curl nor wget found. Cannot download tools."
        fi
        chmod +x "$dest"
    fi
}

# ── Pre-flight ───────────────────────────────────────────────────────────────
need_cmd cmake
need_cmd ninja

info "Building OpenJazz with preset: $PRESET"

# ── CMake configure + build ──────────────────────────────────────────────────
cd "$REPO_ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET"

# Read version from cmake cache (grep for the value after the colon+equals)
OJ_VERSION=$(grep -m1 'CMAKE_PROJECT_VERSION:STATIC=' "$BUILD_DIR/CMakeCache.txt" \
    | cut -d= -f2) || true
[[ -z "$OJ_VERSION" ]] && OJ_VERSION=$(grep 'Version:' <(cmake -L "$BUILD_DIR" 2>/dev/null) \
    | head -1 | awk '{print $NF}') || true
[[ -z "$OJ_VERSION" ]] && OJ_VERSION="20260301"

info "OpenJazz version: $OJ_VERSION"

# ── Install into AppDir ───────────────────────────────────────────────────────
info "Staging AppDir..."
rm -rf "$APPDIR"
cmake --install "$BUILD_DIR" --prefix "$APPDIR/usr"

# ── AppDir metadata ───────────────────────────────────────────────────────────
info "Adding AppDir metadata..."

# Desktop file
mkdir -p "$APPDIR/usr/share/applications"
cp "$REPO_ROOT/res/unix/OpenJazz.desktop" \
   "$APPDIR/usr/share/applications/OpenJazz.desktop"

# Icons (48×48 PNG and scalable SVG)
mkdir -p "$APPDIR/usr/share/icons/hicolor/48x48/apps"
mkdir -p "$APPDIR/usr/share/icons/hicolor/scalable/apps"
mkdir -p "$APPDIR/usr/share/pixmaps"
cp "$REPO_ROOT/res/unix/OpenJazz.png" \
   "$APPDIR/usr/share/icons/hicolor/48x48/apps/OpenJazz.png"
cp "$REPO_ROOT/res/unix/OpenJazz.svg" \
   "$APPDIR/usr/share/icons/hicolor/scalable/apps/OpenJazz.svg"
cp "$REPO_ROOT/res/unix/OpenJazz.png" \
   "$APPDIR/usr/share/pixmaps/OpenJazz.png"

# Symlinks required by AppImage spec
cp "$REPO_ROOT/res/unix/OpenJazz.png" "$APPDIR/OpenJazz.png"
cp "$REPO_ROOT/res/unix/OpenJazz.desktop" "$APPDIR/OpenJazz.desktop"

# ── Custom AppRun ─────────────────────────────────────────────────────────────
# Built with PORTABLE=ON: OpenJazz reads/writes openjazz.cfg from CWD only.
# We always cd to the AppImage directory so the config file placed next to the
# AppImage is found and used (and new saves go back there, not to ~/home/deck).
#
# Game data lookup order (when no explicit path arg is given):
#   1. AppImage directory (flat layout: PANEL.000 next to .AppImage)
#   2. Any direct subdirectory of the AppImage folder containing PANEL.000
#   3. Game's own built-in search paths
cat > "$APPDIR/AppRun" << 'APPRUN_EOF'
#!/usr/bin/env bash
# AppRun — OpenJazz launcher for AppImage
#
# Built with PORTABLE=ON: config (openjazz.cfg) is read/written from CWD.
# We cd to the AppImage directory so the bundled openjazz.cfg is found and
# settings persist there across sessions.
#
# Game data (PANEL.000, etc.) can be found automatically when placed next to
# the AppImage, or in a subfolder. Or pass a path: ./OpenJazz.AppImage /path

BINARY="$APPDIR/usr/bin/OpenJazz"

# cd to AppImage dir so PORTABLE config (openjazz.cfg) is found/saved there
if [[ -n "${APPIMAGE:-}" ]]; then
    cd "$(dirname "$APPIMAGE")"
fi

if [[ $# -gt 0 ]]; then
    # User provided explicit path(s) — forward them directly
    exec "$BINARY" "$@"
elif [[ -f "$PWD/PANEL.000" ]]; then
    # Game data found in AppImage directory (flat layout)
    exec "$BINARY" "$PWD"
else
    # Search direct subdirectories for PANEL.000
    for subdir in "$PWD"/*/; do
        if [[ -f "$subdir/PANEL.000" ]]; then
            exec "$BINARY" "$subdir"
        fi
    done
    # No data found — let the game try its default paths
    exec "$BINARY"
fi
APPRUN_EOF
chmod +x "$APPDIR/AppRun"

# ── Download packaging tools ──────────────────────────────────────────────────
APPIMAGETOOL="$TOOLS_DIR/appimagetool-x86_64.AppImage"
download_tool "$APPIMAGETOOL_URL"  "$APPIMAGETOOL"
export APPIMAGE_EXTRACT_AND_RUN=1

# Strip .note.gnu.property section to remove x86-64-v4 ISA requirement injected
# by the host system's crt1.o (this Gentoo build machine targets x86-64-v4).
# The binary itself has no v4 instructions; removing the note lets glibc on
# SteamOS load it without complaining about ISA level.
info "Stripping .note.gnu.property from binary..."
objcopy --remove-section=.note.gnu.property "$APPDIR/usr/bin/OpenJazz"

# ── Skip library bundling ─────────────────────────────────────────────────────
# All system libs (SDL2, PulseAudio, X11, FLAC, etc.) on this build machine are
# compiled for x86-64-v4 (AVX-512). Bundling them would crash on the Steam Deck
# (Zen 2 / x86-64-v3). SteamOS provides all these libs in v3-compatible form,
# so we package the binary only and rely on system-provided libs at runtime.
info "Skipping lib bundling — relying on SteamOS system libs (SDL2, PulseAudio, etc.)"
[[ -f "$APPDIR/usr/bin/OpenJazz" ]] || error "Binary missing from AppDir"

# ── Create AppImage ───────────────────────────────────────────────────────────
OUTPUT="$REPO_ROOT/openjazz.appimage"
info "Creating AppImage: $OUTPUT"

"$APPIMAGETOOL" --no-appstream "$APPDIR" "$OUTPUT"

chmod +x "$OUTPUT"

info "Done! AppImage: $OUTPUT"
cat << 'USAGE_EOF'

Usage on Steam Deck:
  1. Place Jazz Jackrabbit 1 game data in the same folder as the AppImage.
  2. Mark executable: chmod +x openjazz.appimage
  3. Run: ./openjazz.appimage
     Or pass a data path: ./openjazz.appimage /path/to/jazz1

     First launch: 1280×800 fullscreen, 2× scale, bilinear filtering.
     Press Escape → Setup Options → Video to change resolution/scaling.
USAGE_EOF
