#!/usr/bin/env python3
"""
extract_scenes.py — Extract animation frames from Jazz Jackrabbit 1 .0SC cutscene files

Reads every .0SC file in the game directory, decompresses the delta-encoded
animation frames, applies the embedded palette, and saves each composited
frame as a RGBA PNG suitable for hi-res upscaling.

Output layout:
    <output_dir>/
        STARTUP.0SC/
            anim_000/
                frame_0000.png
                frame_0001.png
                ...
        END.0SC/
            anim_001/
                frame_0000.png
                ...

Usage:
    python3 tools/extract_scenes.py <game_dir> [output_dir]

    game_dir   — directory containing *.0SC files (e.g. your Jazz 1 game folder)
    output_dir — where to write PNGs  (default: extracted_scenes)

Requirements:
    pip install Pillow
"""

import os
import sys
import struct
from pathlib import Path
from typing import List, Optional, Tuple

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

# ── Constants ──────────────────────────────────────────────────────────────────

SW, SH = 320, 200          # Original screen dimensions
MAX_COLORS = 256           # Palette size


def _hdr(a: str, b: str) -> int:
    """MAKE_0SC_HEADER(a, b) — little-endian u16 from two ASCII chars."""
    return ord(a) | (ord(b) << 8)


# ANI block type constants (match jj1scene.h)
ANI_HEADER  = _hdr('A', 'N')   # 0x4E41 — marks an animation data block
H11         = _hdr('1', '1')   # raw background image (SW*SH raw bytes)
H1L         = _hdr('1', 'L')   # compacted-compressed background
H_FF        = _hdr('F', 'F')   # FullFrame delta frame
H_SQBR      = _hdr(']', '[')   # Compacted delta frame  (']' | '[' << 8)
H_RC        = _hdr('R', 'C')   # RC delta (not implemented in OpenJazz)
H_RR        = _hdr('R', 'R')   # reverse-animation flag
H_END       = _hdr('_', 'E')   # end-of-animation marker
H_SL        = _hdr('S', 'L')   # sound list
H_ST        = _hdr('S', 'T')   # sound tag
H_PL        = _hdr('P', 'L')   # playlist (palette + frame blocks)


# ── Low-level helpers ──────────────────────────────────────────────────────────

def u16le(data: bytes, pos: int) -> int:
    return data[pos] | (data[pos + 1] << 8)


def s32le(data: bytes, pos: int) -> int:
    v = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24)
    return v - 0x100000000 if v >= 0x80000000 else v


def load_null_string(data: bytes, pos: int) -> Tuple[str, int]:
    """Read a null-terminated string; return (string, new_pos)."""
    end = data.index(0, pos)
    return data[pos:end].decode('latin-1', errors='replace'), end + 1


def load_palette(data: bytes, pos: int) -> Tuple[List[Tuple[int,int,int,int]], int]:
    """
    Load 256-colour 6-bit palette (768 bytes, raw, no RLE).
    Returns list of (r,g,b,255) tuples scaled to 8-bit, and new position.
    Matches File::loadPalette(palette, rle=false) in OpenJazz.
    """
    colors: List[Tuple[int,int,int,int]] = []
    for i in range(MAX_COLORS):
        r6 = data[pos + i * 3]
        g6 = data[pos + i * 3 + 1]
        b6 = data[pos + i * 3 + 2]
        # 6-bit → 8-bit: (v << 2) | (v >> 4)
        colors.append((
            (r6 << 2) | (r6 >> 4),
            (g6 << 2) | (g6 >> 4),
            (b6 << 2) | (b6 >> 4),
            255,
        ))
    return colors, pos + MAX_COLORS * 3


# ── Decompression algorithms ───────────────────────────────────────────────────

def load_compacted_mem(frame_data: bytes, pixels: bytearray) -> None:
    """
    Decompress an ECompactedAniHeader / E1LAniHeader block into *pixels*.

    Column-fill compression: each command advances the current horizontal
    position by fill_width and optionally paints a vertical stripe of that
    width downward through all remaining rows.

    Faithfully mirrors JJ1Scene::loadCompactedMem() in jj1sceneload.cpp.
    """
    end = SW * SH
    pos = 0
    pixel_pos = 0

    while pos < len(frame_data) and pixel_pos < end:
        header = frame_data[pos]; pos += 1
        fill_width = 0

        if header == 0x7F:
            # Wide fill: u16LE column count + 1-byte fill colour
            if pos + 3 > len(frame_data):
                break
            fill_width = frame_data[pos] | (frame_data[pos + 1] << 8); pos += 2
            fill_color = frame_data[pos]; pos += 1
            fp = pixel_pos
            while fp + fill_width <= end:
                pixels[fp:fp + fill_width] = bytes([fill_color]) * fill_width
                fp += SW
            if fp < end:
                pixels[fp:end] = bytes([fill_color]) * (end - fp)

        elif header == 0xFF:
            # Wide skip: u16LE column count, no write
            if pos + 2 > len(frame_data):
                break
            fill_width = frame_data[pos] | (frame_data[pos + 1] << 8); pos += 2

        elif header & 0x80:
            # Short skip: 1–127 columns
            fill_width = (header - 0x80) + 1

        elif header & 0x40:
            # Short fill: 1–63 columns with 1-byte colour
            if pos >= len(frame_data):
                break
            fill_color = frame_data[pos]; pos += 1
            fill_width = (header - 0x40) + 1
            fp = pixel_pos
            while fp + fill_width <= end:
                pixels[fp:fp + fill_width] = bytes([fill_color]) * fill_width
                fp += SW
            if fp < end:
                pixels[fp:end] = bytes([fill_color]) * (end - fp)

        else:
            # Raw columns: 1–64 columns, each pixel written down its column
            fill_width = (header & 0x3F) + 1
            for col in range(fill_width):
                if pos >= len(frame_data):
                    break
                color = frame_data[pos]; pos += 1
                if color != 0xFF:
                    fp = pixel_pos + col
                    while fp < end:
                        pixels[fp] = color
                        fp += SW

        pixel_pos += fill_width


def load_full_frame_mem(frame_data: bytes, pixels: bytearray) -> None:
    """
    Decompress an EFullFrameAniHeader block into *pixels*.

    Pixel-level delta compression that alternates between transparent (skip)
    and opaque (copy/fill) runs.  The 'trans' flag toggles with each command
    unless bit 7 of the header is set (which forces opaque mode).

    Faithfully mirrors JJ1Scene::loadFullFrameMem() in jj1sceneload.cpp.
    """
    end = SW * SH
    pos = 0
    pixel_pos = 0
    trans = True

    while pos < len(frame_data) and pixel_pos < end:
        header = frame_data[pos]; pos += 1
        fill_width = 0

        if (header & 0x7F) == 0x7F:
            # Wide run: u16LE count (transparent adds 255 to it)
            if pos + 2 > len(frame_data):
                break
            fill_width = frame_data[pos] | (frame_data[pos + 1] << 8); pos += 2
            if trans:
                fill_width += 255
            # Skip only — no pixel write regardless of trans

        elif header:
            if trans:
                # Transparent run: skip 'header' pixels
                fill_width = header
            else:
                fill_width = header & 0x1F
                op = header & 0x60
                # Clamp write to buffer end — advancing pixel_pos by fill_width
                # regardless so the stream stays aligned (C++ does the same).
                w = min(fill_width, end - pixel_pos)

                if op == 0x00:
                    # Copy fill_width pixels from stream
                    if pos + fill_width > len(frame_data):
                        break
                    if w > 0:
                        pixels[pixel_pos:pixel_pos + w] = \
                            frame_data[pos:pos + w]
                    pos += fill_width

                elif op == 0x20:
                    # Copy fill_width pixels from the previous row
                    if w > 0 and pixel_pos >= SW:
                        src = pixel_pos - SW
                        pixels[pixel_pos:pixel_pos + w] = \
                            pixels[src:src + w]

                elif op == 0x60:
                    # Fill (extended range: bits 0–5)
                    fill_width = header & 0x3F
                    w = min(fill_width, end - pixel_pos)
                    if pos >= len(frame_data):
                        break
                    if w > 0:
                        pixels[pixel_pos:pixel_pos + w] = \
                            bytes([frame_data[pos]]) * w
                    pos += 1

                elif op == 0x40:
                    # Fill (normal range: bits 0–4)
                    if pos >= len(frame_data):
                        break
                    if w > 0:
                        pixels[pixel_pos:pixel_pos + w] = \
                            bytes([frame_data[pos]]) * w
                    pos += 1

        else:
            # header == 0: end-of-stream marker
            return

        pixel_pos += fill_width

        if header & 0x80:
            trans = False
        else:
            trans = not trans


# ── Scene data structures ──────────────────────────────────────────────────────

class SceneAnim:
    """One parsed animation block from a .0SC file."""

    def __init__(self, anim_id: int,
                 palette: List[Tuple[int,int,int,int]],
                 background: Optional[bytearray],
                 frames: List[Tuple[int, bytes]]) -> None:
        self.id         = anim_id
        self.palette    = palette    # 256 × (r,g,b,255)
        self.background = background # bytearray(SW*SH) or None
        self.frames     = frames     # list of (frame_type, raw_bytes)


# ── .0SC parser ───────────────────────────────────────────────────────────────

def _parse_animation(data: bytes, pos: int, anim_id: int) -> Optional[SceneAnim]:
    """
    Parse a single animation from the .0SC data block.
    *pos* points just after the ANI_HEADER short.
    Returns a SceneAnim or None if no PL block (palette) was found.
    """
    # Two u16 fields before the type loop: DataLen and frame-count hint
    pos += 4

    palette: Optional[List] = None
    background: Optional[bytearray] = None
    frames: List[Tuple[int, bytes]] = []

    while pos + 2 <= len(data):
        type_val = u16le(data, pos); pos += 2

        if type_val == H_SL:
            # Sound list — skip all sound name strings
            if pos + 3 > len(data):
                break
            pos += 2  # offset short
            n_sounds = data[pos]; pos += 1
            for _ in range(n_sounds):
                _, pos = load_null_string(data, pos)

        elif type_val == H_PL:
            # Playlist block — contains palette + animation blocks
            if pos + 2 > len(data):
                break
            pos += 2  # block length short (unused; we process until H_END)

            palette, pos = load_palette(data, pos)

            # Process animation sub-blocks: [u16 type][u16 size][data…]
            while pos + 4 <= len(data):
                blk_type = u16le(data, pos); pos += 2
                blk_size = u16le(data, pos); pos += 2
                next_pos = pos + blk_size

                if blk_type == H_END:
                    pos = next_pos
                    break

                elif blk_type == H11:
                    # Raw background: SW×SH bytes
                    if pos + SW * SH <= len(data):
                        background = bytearray(data[pos:pos + SW * SH])

                elif blk_type == H1L:
                    # Compacted-compressed background (start from zeros)
                    bg = bytearray(SW * SH)
                    load_compacted_mem(data[pos:pos + blk_size], bg)
                    background = bg

                elif blk_type == H_FF:
                    frames.append((H_FF,   bytes(data[pos:pos + blk_size])))

                elif blk_type == H_SQBR:
                    frames.append((H_SQBR, bytes(data[pos:pos + blk_size])))

                elif blk_type == H_RC:
                    # RC frames are stored but not played back by OpenJazz;
                    # record as placeholder so frame indices stay aligned.
                    frames.append((H_RC,   bytes(data[pos:pos + blk_size])))

                # H_RR (reverse flag), H_ST (sound tag), unknown types — skip

                pos = next_pos

            break  # PL block found and processed

        # Any other pre-PL type — skip (shouldn't appear but be safe)

    if palette is None:
        return None

    return SceneAnim(anim_id, palette, background, frames)


def parse_scene_file(data: bytes) -> List[SceneAnim]:
    """
    Parse a complete .0SC file and return all SceneAnim objects found.

    File layout (from JJ1Scene constructor + loadData):
        [18]  "Digital Dimensions"
        [1]   0x1A
        [4]   dataOffset (s32LE) — offset to data-item table
        [2]   scriptItems (u16LE)
        [scriptItems×4]  scriptStart offsets (unused here)
        --- seek to dataOffset ---
        [2]   N (u16LE); dataItems = N + 1
        [dataItems×4]  dataOffset[] (s32LE)
        --- for each data item: seek to dataOffset[i] ---
        [2]   dataLen (u16LE)
        if dataLen == ANI_HEADER: animation block
        else: image or palette (not needed for frame extraction)
    """
    if len(data) < 24:
        raise ValueError("File too short")
    if data[0:18] != b'Digital Dimensions' or data[18] != 0x1A:
        raise ValueError("Not a valid .0SC file (bad header)")

    pos = 19
    data_offset = s32le(data, pos); pos += 4
    script_items = u16le(data, pos); pos += 2
    pos += script_items * 4   # skip scriptStart[] table

    # Seek to data-item table
    pos = data_offset
    if pos + 2 > len(data):
        return []
    data_items = u16le(data, pos) + 1; pos += 2

    data_offsets: List[int] = []
    for _ in range(data_items):
        if pos + 4 > len(data):
            break
        data_offsets.append(s32le(data, pos)); pos += 4

    animations: List[SceneAnim] = []
    for item_idx, offset in enumerate(data_offsets):
        if offset < 0 or offset + 2 > len(data):
            continue
        item_pos = offset
        data_len = u16le(data, item_pos); item_pos += 2

        if data_len == ANI_HEADER:
            anim = _parse_animation(data, item_pos, item_idx)
            if anim is not None:
                animations.append(anim)

    return animations


# ── Frame rendering ────────────────────────────────────────────────────────────

def apply_palette(pixels: bytearray,
                  palette: List[Tuple[int,int,int,int]]) -> bytes:
    """Convert an 8-bit indexed pixel buffer to RGBA bytes."""
    rgba = bytearray(SW * SH * 4)
    for i in range(SW * SH):
        r, g, b, a = palette[pixels[i]]
        rgba[i * 4]     = r
        rgba[i * 4 + 1] = g
        rgba[i * 4 + 2] = b
        rgba[i * 4 + 3] = a
    return bytes(rgba)


# ── Main extraction logic ──────────────────────────────────────────────────────

def extract_scenes(game_dir: Path, output_dir: Path) -> int:
    """
    Extract all .0SC scene animations from *game_dir* into *output_dir*.
    Returns the total number of PNG frames written.
    """
    # Collect .0SC files (case-insensitive on Linux)
    scene_files = sorted(
        p for p in game_dir.iterdir()
        if p.is_file() and p.suffix.upper() == '.0SC'
    )
    if not scene_files:
        print(f"No .0SC files found in {game_dir}")
        return 0

    total_frames = 0

    for scene_path in scene_files:
        scene_name = scene_path.name.upper()
        print(f"\n{scene_name}")

        try:
            raw = scene_path.read_bytes()
        except OSError as exc:
            print(f"  Read error: {exc}")
            continue

        try:
            animations = parse_scene_file(raw)
        except (ValueError, IndexError) as exc:
            print(f"  Parse error: {exc}")
            continue

        if not animations:
            print("  No animations found.")
            continue

        for anim in animations:
            anim_dir = output_dir / scene_name / f"anim_{anim.id:03d}"
            anim_dir.mkdir(parents=True, exist_ok=True)

            # Accumulated scratch buffer (starts as background or zeros)
            scratch = bytearray(SW * SH)
            if anim.background:
                scratch[:] = anim.background

            n_frames = len(anim.frames)
            print(f"  anim_{anim.id:03d}: {n_frames} frame(s) ...", end=" ", flush=True)

            written = 0
            for frame_idx, (frame_type, frame_bytes) in enumerate(anim.frames):
                if frame_type == H_FF:
                    load_full_frame_mem(frame_bytes, scratch)
                elif frame_type == H_SQBR:
                    load_compacted_mem(frame_bytes, scratch)
                elif frame_type == H_RC:
                    pass  # not implemented in OpenJazz playback; keep scratch unchanged

                rgba = apply_palette(scratch, anim.palette)
                img = Image.frombytes('RGBA', (SW, SH), rgba)
                img.save(str(anim_dir / f"frame_{frame_idx:04d}.png"))
                written += 1

            total_frames += written
            print(f"saved {written}")

    return total_frames


# ── Entry point ────────────────────────────────────────────────────────────────

def main() -> None:
    import argparse
    parser = argparse.ArgumentParser(
        description="Extract Jazz Jackrabbit 1 .0SC cutscene frames to PNG sequences"
    )
    parser.add_argument("game_dir",
                        help="Game data directory containing .0SC files")
    parser.add_argument("output_dir", nargs="?", default="extracted_scenes",
                        help="Output directory (default: extracted_scenes)")
    args = parser.parse_args()

    game_dir = Path(args.game_dir)
    if not game_dir.is_dir():
        sys.exit(f"Game directory not found: {game_dir}")

    output_dir = Path(args.output_dir)

    total = extract_scenes(game_dir, output_dir)
    if total > 0:
        print(f"\nDone. {total} frames written to {output_dir}/")
        print()
        print("Next steps:")
        print("  1. Upscale the frames:")
        print(f"     python3 tools/upscale_assets.py {output_dir} hires_scenes")
        print("  2. Copy hires_scenes/ contents into your hires/ folder")
        print("     (place hires/scenes/ next to your game data files)")
        print("  3. Rebuild OpenJazz with -DHIRES=ON")


if __name__ == "__main__":
    main()
