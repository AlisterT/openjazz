#!/usr/bin/env python3
"""
extract_assets.py — Jazz Jackrabbit 1 asset extractor

Extracts tile graphics and sprites from Jazz Jackrabbit 1 game data files,
saving them as PNG images suitable for use as hi-res replacement assets.

Usage:
    python3 tools/extract_assets.py /path/to/jazz1 [output_dir]

Output structure:
    output_dir/
      tiles/
        BLOCKS_000_sheet.png       ← full tile sheet (16 across)
        BLOCKS_000/                ← individual 32×32 tiles
          tile_0000.png
          tile_0001.png
          ...
      sprites/
        SPRITES_000_sheet.png
        SPRITES_000/
          sprite_0000.png
          ...

Requirements:
    pip install Pillow
"""

import sys
import struct
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

# ── Constants (must match OpenJazz source) ─────────────────────────────────────
TILE_SIZE  = 32    # TTOI(1) — tile width/height in pixels
TILE_KEY   = 127   # TKEY  — transparent palette index for tiles
SPRITE_KEY = 254   # SKEY  — transparent palette index for sprites
TNUM       = 60    # tiles per tileset section
TSETS      = 4     # tileset sections per BLOCKS file
MAX_COLORS = 256


# ── Low-level file reader ──────────────────────────────────────────────────────

class JazzFile:
    """
    Mirrors the OpenJazz File class interface against an in-memory byte buffer.
    Method names match the C++ originals so the extraction logic reads the same.
    """

    def __init__(self, path: Path):
        self._data = path.read_bytes()
        self._pos  = 0
        self.size  = len(self._data)

    def tell(self) -> int:
        return self._pos

    def seek(self, offset: int, reset: bool = False):
        if reset:
            self._pos = offset
        else:
            self._pos += offset

    # ── Primitive readers ──────────────────────────────────────────────────────

    def load_char(self) -> int:
        v = self._data[self._pos]
        self._pos += 1
        return v

    def load_short(self, max_val: int = 0xFFFF) -> int:
        v = self._data[self._pos] | (self._data[self._pos + 1] << 8)
        self._pos += 2
        return min(v, max_val)

    def load_block(self, length: int) -> bytes:
        v = self._data[self._pos:self._pos + length]
        self._pos += length
        return v

    # ── RLE ───────────────────────────────────────────────────────────────────

    def load_rle(self, length: int, check_size: bool = True) -> bytes:
        """
        Decode RLE-compressed data.
        Mirrors File::loadRLE(length, checkSize=true).

        When check_size=True (default): reads a 2-byte LE size prefix first,
        then seeks to start+size after decoding (handles padding/alignment).

        Encoding:
          code = byte; amount = code & 0x7F
          if code & 0x80  → repeat `amount` copies of next byte
          elif amount     → copy next `amount` bytes literally
          else (end)      → store one more byte, then stop
        """
        if check_size:
            size      = self._data[self._pos] | (self._data[self._pos + 1] << 8)
            self._pos += 2
            start      = self._pos

        buf = bytearray(length)
        pos = 0
        d   = self._data

        while pos < length and self._pos < self.size:
            code   = d[self._pos]; self._pos += 1
            amount = code & 0x7F

            if code & 0x80:                  # repeat run
                value = d[self._pos]; self._pos += 1
                end   = min(pos + amount, length)
                buf[pos:end] = bytes([value]) * (end - pos)
                pos  += amount

            elif amount:                     # literal copy
                end   = min(pos + amount, length)
                n     = end - pos
                buf[pos:end] = d[self._pos:self._pos + n]
                self._pos   += amount
                pos          = end

            else:                            # end marker — one extra byte
                if self._pos < self.size:
                    buf[pos] = d[self._pos]; self._pos += 1
                pos += 1
                break

        if check_size:
            self._pos = start + size   # seek to exact end (handles any padding)

        return bytes(buf)

    def skip_rle(self):
        """
        Skip an RLE block using its 2-byte LE size prefix.
        Mirrors File::skipRLE().
        """
        size      = self._data[self._pos] | (self._data[self._pos + 1] << 8)
        self._pos += 2 + size

    def load_string(self, length: int) -> bytes:
        return self.load_block(length)

    # ── Palette ───────────────────────────────────────────────────────────────

    def load_palette(self) -> list:
        """
        Load 256-colour palette from RLE-encoded data (no size prefix).
        Colours are 6-bit per channel, expanded to 8-bit.
        Mirrors File::loadPalette(palette, rle=true).
        """
        raw     = self.load_rle(MAX_COLORS * 3)
        palette = []
        for i in range(MAX_COLORS):
            r, g, b = raw[i * 3], raw[i * 3 + 1], raw[i * 3 + 2]
            # 6-bit → 8-bit: value<<2 | value>>4
            palette.append((
                ((r << 2) | (r >> 4)) & 0xFF,
                ((g << 2) | (g >> 4)) & 0xFF,
                ((b << 2) | (b >> 4)) & 0xFF,
            ))
        return palette

    # ── Pixel unscrambling ────────────────────────────────────────────────────

    def load_pixels(self, length: int) -> bytes:
        """
        Load scrambled (unmasked) pixel data.
        Mirrors File::loadPixels(length) — no mask, just de-interleave.
        """
        raw     = self.load_block(length)
        out     = bytearray(length)
        quarter = length >> 2
        for i in range(length):
            out[i] = raw[(i >> 2) + ((i & 3) * quarter)]
        return bytes(out)

    def load_pixels_masked(self, length: int, key: int) -> bytes:
        """
        Load scrambled + masked pixel data.
        Mirrors File::loadPixels(length, key).

        Step 1: read 1-bit mask (4 pixels per byte, LSB first)
        Step 2: scramble mask the same way pixels are scrambled
        Step 3: read actual pixel bytes only where mask==1 (skip key value)
        Step 4: descramble pixels into display order
        """
        d       = self._data
        quarter = length >> 2

        # Step 1 — read mask bits into pixels[] (using pixels as temp)
        pixels = bytearray(length)
        mask_byte = 0
        for i in range(length):
            if not (i & 3):
                mask_byte = d[self._pos]; self._pos += 1
            pixels[i] = (mask_byte >> (i & 3)) & 1

        # Step 2 — scramble the mask into sorted[]
        sorted_arr = bytearray(length)
        for i in range(length):
            sorted_arr[(i >> 2) + ((i & 3) * quarter)] = pixels[i]

        # Step 3 — read pixel values for opaque positions
        for i in range(length):
            pixels[i] = key
            if sorted_arr[i]:
                while True:
                    pixels[i] = d[self._pos]; self._pos += 1
                    if pixels[i] != key:
                        break

        # Step 4 — descramble into sorted[]
        for i in range(length):
            sorted_arr[i] = pixels[(i >> 2) + ((i & 3) * quarter)]

        return bytes(sorted_arr)


# ── Image helpers ─────────────────────────────────────────────────────────────

def to_rgba(pixels: bytes, w: int, h: int, palette: list, key: int) -> Image.Image:
    img  = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    pix  = img.load()
    for y in range(h):
        row = y * w
        for x in range(w):
            idx = pixels[row + x]
            if idx != key:
                r, g, b = palette[idx]
                pix[x, y] = (r, g, b, 255)
    return img


def make_sheet(images: list, tile_w: int, tile_h: int, cols: int = 16) -> Image.Image:
    rows   = (len(images) + cols - 1) // cols
    sheet  = Image.new('RGBA', (cols * tile_w, rows * tile_h), (0, 0, 0, 0))
    for i, img in enumerate(images):
        sheet.paste(img, ((i % cols) * tile_w, (i // cols) * tile_h))
    return sheet


# ── Tile extractor ─────────────────────────────────────────────────────────────

def extract_blocks(src: Path, out_dir: Path):
    """Extract all tiles from a BLOCKS.xxx file."""
    f   = JazzFile(src)
    ext = src.suffix.lstrip('.')

    # Load palettes (the third uses a size-prefixed RLE, so skip with skip_rle)
    palette = f.load_palette()
    f.load_palette()  # sky palette — skip
    f.skip_rle()      # second sky palette — has size prefix

    tile_dir = out_dir / f'BLOCKS_{ext}'
    tile_dir.mkdir(parents=True, exist_ok=True)

    tiles = []
    idx   = 0

    for section in range(TSETS):
        if f.tell() + 2 > f.size:
            break
        marker = f.load_string(2)
        if marker != b'ok':
            continue   # empty section ('  '), no tile data follows

        for _ in range(TNUM):
            raw = f.load_rle(TILE_SIZE * TILE_SIZE)
            # Tiles use plain RLE — no pixel scrambling
            img = to_rgba(raw, TILE_SIZE, TILE_SIZE, palette, TILE_KEY)
            img.save(str(tile_dir / f'tile_{idx:04d}.png'))
            tiles.append(img)
            idx += 1

    if tiles:
        sheet = make_sheet(tiles, TILE_SIZE, TILE_SIZE)
        sheet.save(str(out_dir / f'BLOCKS_{ext}_sheet.png'))

    print(f"  {src.name}: {idx} tiles → {tile_dir.name}/")
    return palette  # caller can reuse this palette for sprites


# ── Sprite extractor ──────────────────────────────────────────────────────────

def load_one_sprite(f: JazzFile, palette: list) -> Image.Image | None:
    """
    Read one sprite from JazzFile at current position.
    Mirrors JJ1Level::loadSprite().
    Returns RGBA image or None for empty/zero-size sprites.
    """
    if f.tell() + 10 > f.size:
        return None

    width       = f.load_short() << 2
    height      = f.load_short()
    f.seek(2)                        # skip 2 bytes
    mask_offset = f.load_short()
    pos         = f.load_short() << 2

    if width == 0:
        return None

    if mask_offset:
        # Masked sprite
        height += 1
        f.seek(mask_offset)          # seek forward to mask data
        # pos is now the absolute end offset (skip target after pixels)
        end_pos = f.tell() + (width >> 2) * height + pos
        pixels  = f.load_pixels_masked(width * height, SPRITE_KEY)
        f.seek(end_pos, reset=True)
    else:
        pixels = f.load_pixels(width * height)

    return to_rgba(pixels, width, height, palette, SPRITE_KEY)


def extract_sprites(spec_path: Path, main_path: Path,
                    palette: list, out_dir: Path):
    """
    Extract sprites from a SPRITES.xxx + MAINCHAR.000 pair.
    Mirrors JJ1Level::loadSprites().
    """
    spec = JazzFile(spec_path)
    main = JazzFile(main_path)

    ext     = spec_path.suffix.lstrip('.')
    spr_dir = out_dir / f'SPRITES_{ext}'
    spr_dir.mkdir(parents=True, exist_ok=True)

    n_sprites = spec.load_short(256)

    # Offsets table: n_sprites bytes x-offset + n_sprites bytes y-offset
    spec.seek(n_sprites * 2)         # skip offset table

    # MAINCHAR.000: skip 2-byte header
    main.seek(2, reset=True)

    images = []
    idx    = 0

    for i in range(n_sprites):
        img = None

        # Try main file
        if main.tell() < main.size:
            flag = main.load_char()
            if flag == 0xFF:
                main.seek(1)         # skip one more byte
            else:
                main.seek(-1)        # back up, re-read as sprite header
                img = load_one_sprite(main, palette)

        # Try spec file (may override main)
        if spec.tell() < spec.size:
            flag = spec.load_char()
            if flag == 0xFF:
                spec.seek(1)
            else:
                spec.seek(-1)
                sp = load_one_sprite(spec, palette)
                if sp is not None:
                    img = sp                 # spec file wins

        if img is not None:
            img.save(str(spr_dir / f'sprite_{idx:04d}.png'))
            images.append(img)
        idx += 1

        if spec.tell() >= spec.size:
            break

    if images:
        sheet = make_sheet(images, TILE_SIZE, TILE_SIZE, cols=16)
        sheet.save(str(out_dir / f'SPRITES_{ext}_sheet.png'))

    print(f"  {spec_path.name}: {len(images)} sprites → {spr_dir.name}/")


# ── Entry point ───────────────────────────────────────────────────────────────

def find_file(directory: Path, stem: str) -> Path | None:
    """Case-insensitive file lookup."""
    for p in directory.iterdir():
        if p.name.upper() == stem.upper():
            return p
    return None


def main():
    if len(sys.argv) < 2:
        sys.exit(f"Usage: {sys.argv[0]} <game_data_dir> [output_dir]")

    game_dir = Path(sys.argv[1])
    out_dir  = Path(sys.argv[2]) if len(sys.argv) > 2 else Path('extracted_assets')

    if not game_dir.is_dir():
        sys.exit(f"Directory not found: {game_dir}")

    tiles_dir   = out_dir / 'tiles'
    sprites_dir = out_dir / 'sprites'
    tiles_dir.mkdir(parents=True, exist_ok=True)
    sprites_dir.mkdir(parents=True, exist_ok=True)

    # ── Tiles ──────────────────────────────────────────────────────────────────
    print("Extracting tiles...")
    palettes = {}   # ext → palette (for reuse with sprites)

    seen_blocks = {}
    for f in sorted(game_dir.iterdir()):
        key = f.name.upper()
        if key.startswith('BLOCKS.') and key not in seen_blocks:
            seen_blocks[key] = f

    for key in sorted(seen_blocks):
        f = seen_blocks[key]
        try:
            pal = extract_blocks(f, tiles_dir)
            ext = f.suffix.lstrip('.')
            palettes[ext] = pal
        except Exception as e:
            print(f"  {f.name}: ERROR — {e}")

    # ── Sprites ────────────────────────────────────────────────────────────────
    print("\nExtracting sprites...")
    main_file = find_file(game_dir, 'MAINCHAR.000')
    if not main_file:
        print("  MAINCHAR.000 not found — skipping sprites")
    else:
        seen_sprites = {}
        for f in sorted(game_dir.iterdir()):
            key = f.name.upper()
            if key.startswith('SPRITES.') and key not in seen_sprites:
                seen_sprites[key] = f

        # Pick a default palette (BLOCKS.000 if available)
        default_pal = palettes.get('000') or (list(palettes.values())[0] if palettes else
                      [(i, i, i) for i in range(256)])

        for key in sorted(seen_sprites):
            f = seen_sprites[key]
            ext = f.suffix.lstrip('.')
            pal = palettes.get(ext, default_pal)
            try:
                extract_sprites(f, main_file, pal, sprites_dir)
            except Exception as e:
                print(f"  {f.name}: ERROR — {e}")

    print(f"\nDone. Assets written to: {out_dir}/")


if __name__ == '__main__':
    main()
