#!/usr/bin/env bash
# build_appimage.sh — Build OpenJazz AppImage for Steam Deck (Linux x86_64)
#
# Usage:
#   ./builds/steamdeck/build_appimage.sh
#
# Output: OpenJazz-<version>-steamdeck-x86_64.AppImage
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
# Priority order for game data lookup (when no explicit path arg is given):
#   1. Current working directory (allows: cd ~/jazz1 && ./OpenJazz.AppImage)
#   2. Directory containing the .AppImage file
#   3. Game's own built-in search paths (executable dir, XDG dirs, etc.)
cat > "$APPDIR/AppRun" << 'APPRUN_EOF'
#!/usr/bin/env bash
# AppRun — OpenJazz launcher for AppImage
#
# Game data (PANEL.000, fonts, levels …) can be found automatically when:
#   • Run from a directory containing game data: cd ~/jazz1 && ./OpenJazz.AppImage
#   • Game data is in the same folder as the .AppImage file
# Or pass a path explicitly: ./OpenJazz.AppImage /path/to/jazz1
set -euo pipefail

BINARY="$APPDIR/usr/bin/OpenJazz"

if [[ $# -gt 0 ]]; then
    # User provided explicit path(s) — forward them directly
    exec "$BINARY" "$@"
elif [[ -f "$PWD/PANEL.000" ]]; then
    # Game data found in current working directory
    exec "$BINARY" "$PWD"
elif [[ -n "${APPIMAGE:-}" && -f "$(dirname "$APPIMAGE")/PANEL.000" ]]; then
    # Game data found alongside the .AppImage file
    exec "$BINARY" "$(dirname "$APPIMAGE")"
else
    # No data found in obvious locations — let the game try its default paths
    exec "$BINARY"
fi
APPRUN_EOF
chmod +x "$APPDIR/AppRun"

# ── Download packaging tools ──────────────────────────────────────────────────
LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
APPIMAGETOOL="$TOOLS_DIR/appimagetool-x86_64.AppImage"

download_tool "$LINUXDEPLOY_URL"   "$LINUXDEPLOY"
download_tool "$APPIMAGETOOL_URL"  "$APPIMAGETOOL"

# linuxdeploy needs FUSE or --appimage-extract-and-run
export APPIMAGE_EXTRACT_AND_RUN=1

# Disable stripping: linuxdeploy bundles an older strip that chokes on
# newer ELF .relr.dyn sections produced by current toolchains.
# The libraries work perfectly without strip; they're just slightly larger.
export NO_STRIP=1

# ── Bundle shared libraries ───────────────────────────────────────────────────
info "Bundling shared libraries with linuxdeploy..."
# Allow non-zero exit: strip errors are non-fatal, all libs are still deployed.
"$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/OpenJazz" \
    --desktop-file "$APPDIR/usr/share/applications/OpenJazz.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/48x48/apps/OpenJazz.png" \
    || info "Note: linuxdeploy reported errors (likely strip compat) — continuing."

# Verify the binary and key libs are present before continuing
[[ -f "$APPDIR/usr/bin/OpenJazz" ]]        || error "Binary missing from AppDir"
[[ -f "$APPDIR/usr/lib/libSDL2-2.0.so.0" ]] || error "libSDL2 not deployed"

# ── Create AppImage ───────────────────────────────────────────────────────────
OUTPUT="$REPO_ROOT/OpenJazz-${OJ_VERSION}-steamdeck-x86_64.AppImage"
info "Creating AppImage: $OUTPUT"

"$APPIMAGETOOL" --no-appstream "$APPDIR" "$OUTPUT"

chmod +x "$OUTPUT"

info "Done! AppImage: $OUTPUT"
cat << 'USAGE_EOF'

Usage on Steam Deck:
  1. Place Jazz Jackrabbit 1 game data in the same folder as the AppImage.
  2. Mark executable: chmod +x OpenJazz-*.AppImage
  3. Run: ./OpenJazz-*.AppImage
     Or pass a data path: ./OpenJazz-*.AppImage /path/to/jazz1

     First launch: 1280×800 fullscreen, 2× scale, bilinear filtering.
     Press Escape → Setup Options → Video to change resolution/scaling.
USAGE_EOF
