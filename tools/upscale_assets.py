#!/usr/bin/env python3
"""
upscale_assets.py — AI upscale extracted Jazz Jackrabbit 1 assets

Runs Real-ESRGAN on the PNG files produced by extract_assets.py, creating
4× hi-res versions suitable for use with the OpenJazz hi-res replacement
system.  The upscaler binary is downloaded automatically on first run.

Usage:
    python3 tools/upscale_assets.py [extracted_dir] [hires_dir]

    extracted_dir  — output of extract_assets.py  (default: extracted_assets)
    hires_dir      — where to write 4× PNGs        (default: hires)

Upscaling method (--method flag):
    ai       — Real-ESRGAN (auto-downloaded, best quality, needs Vulkan GPU)
    nearest  — PIL nearest-neighbour 4× (default, pixel-perfect, zero extra deps)
    lanczos  — PIL Lanczos 4× (smooth/blurry, no AI required)

Requirements:
    pip install Pillow
    Vulkan-capable GPU + drivers recommended (for --method ai)
"""

import os
import sys
import shutil
import platform
import subprocess
import tempfile
import zipfile
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

# ── Config ─────────────────────────────────────────────────────────────────────
SCALE       = 4
MODEL       = "realesrgan-x4plus-anime"
ESRGAN_VER  = "v0.2.0"
ESRGAN_URLS = {
    "Linux":   f"https://github.com/xinntao/Real-ESRGAN-ncnn-vulkan/releases/download/{ESRGAN_VER}/realesrgan-ncnn-vulkan-{ESRGAN_VER}-ubuntu.zip",
    "Darwin":  f"https://github.com/xinntao/Real-ESRGAN-ncnn-vulkan/releases/download/{ESRGAN_VER}/realesrgan-ncnn-vulkan-{ESRGAN_VER}-macos.zip",
    "Windows": f"https://github.com/xinntao/Real-ESRGAN-ncnn-vulkan/releases/download/{ESRGAN_VER}/realesrgan-ncnn-vulkan-{ESRGAN_VER}-windows.zip",
}
TOOLS_DIR   = Path(__file__).parent / "bin"


# ── Real-ESRGAN binary management ─────────────────────────────────────────────

def esrgan_binary() -> Path | None:
    """Return path to a working realesrgan-ncnn-vulkan binary, or None."""
    # 1. Already on PATH
    if shutil.which("realesrgan-ncnn-vulkan"):
        return Path(shutil.which("realesrgan-ncnn-vulkan"))

    # 2. Previously downloaded into tools/bin/
    candidates = list(TOOLS_DIR.glob("**/realesrgan-ncnn-vulkan*"))
    candidates += list(TOOLS_DIR.glob("**/realesrgan-ncnn-vulkan.exe"))
    for p in candidates:
        if p.is_file() and not p.suffix == ".zip":
            return p

    return None


def download_esrgan() -> Path | None:
    """Download realesrgan-ncnn-vulkan for the current OS into tools/bin/."""
    os_name = platform.system()
    url = ESRGAN_URLS.get(os_name)
    if not url:
        print(f"  No realesrgan binary available for {os_name}")
        return None

    TOOLS_DIR.mkdir(parents=True, exist_ok=True)
    zip_path = TOOLS_DIR / "realesrgan.zip"

    print(f"  Downloading Real-ESRGAN {ESRGAN_VER} for {os_name}...")
    try:
        if shutil.which("curl"):
            subprocess.run(["curl", "-fsSL", url, "-o", str(zip_path)],
                           check=True, capture_output=True)
        elif shutil.which("wget"):
            subprocess.run(["wget", "-q", url, "-O", str(zip_path)],
                           check=True, capture_output=True)
        else:
            # Pure Python fallback
            import urllib.request
            urllib.request.urlretrieve(url, zip_path)
    except Exception as e:
        print(f"  Download failed: {e}")
        return None

    print("  Extracting...")
    try:
        with zipfile.ZipFile(zip_path) as z:
            z.extractall(TOOLS_DIR)
        zip_path.unlink()
    except Exception as e:
        print(f"  Extraction failed: {e}")
        return None

    # Make binary executable on Unix
    for p in TOOLS_DIR.rglob("realesrgan-ncnn-vulkan*"):
        if p.is_file() and p.suffix not in (".zip", ".md", ".txt"):
            p.chmod(p.stat().st_mode | 0o755)
            return p

    return None


def ensure_esrgan() -> Path | None:
    """Return the esrgan binary path, downloading it first if needed."""
    binary = esrgan_binary()
    if binary:
        return binary
    print("Real-ESRGAN not found — attempting download...")
    return download_esrgan()


# ── Upscaling functions ────────────────────────────────────────────────────────

def upscale_folder_esrgan(binary: Path, src: Path, dst: Path,
                           model_dir: Path) -> bool:
    """
    Run realesrgan-ncnn-vulkan on a directory of PNGs.
    Returns True on success.
    """
    dst.mkdir(parents=True, exist_ok=True)
    try:
        result = subprocess.run(
            [str(binary),
             "-i", str(src),
             "-o", str(dst),
             "-s", str(SCALE),
             "-n", MODEL,
             "-m", str(model_dir),
             "-f", "png"],
            capture_output=True, text=True, timeout=600
        )
        if result.returncode != 0:
            # Vulkan may not be available — show the error once
            print(f"  realesrgan error: {result.stderr.strip()[:200]}")
            return False
        return True
    except subprocess.TimeoutExpired:
        print("  realesrgan timed out")
        return False
    except Exception as e:
        print(f"  realesrgan failed: {e}")
        return False


def upscale_folder_pil(src: Path, dst: Path, resample=Image.NEAREST):
    """
    4× upscale using Pillow with the given resampling filter.
    NEAREST is pixel-perfect; LANCZOS gives smooth (blurry) results.
    """
    dst.mkdir(parents=True, exist_ok=True)
    for png in sorted(src.glob("*.png")):
        img = Image.open(png).convert("RGBA")
        up  = img.resize((img.width * SCALE, img.height * SCALE), resample)
        up.save(str(dst / png.name))


# ── Main ───────────────────────────────────────────────────────────────────────

def main():
    import argparse as _ap
    parser = _ap.ArgumentParser(
        description="Upscale Jazz Jackrabbit 1 extracted assets (tiles and/or scene frames)",
        epilog=(
            "Works on any directory tree produced by extract_assets.py or extract_scenes.py.\n"
            "Typical usage:\n"
            "  Tiles:   python3 tools/upscale_assets.py extracted_assets hires\n"
            "  Scenes:  python3 tools/upscale_assets.py extracted_scenes hires/scenes\n"
            "  Both:    run both commands above"
        ),
    )
    parser.add_argument("extracted_dir", nargs="?", default="extracted_assets",
                        help="Directory of extracted PNGs (default: extracted_assets)")
    parser.add_argument("hires_dir", nargs="?", default="hires",
                        help="Where to write 4× PNGs (default: hires)")
    parser.add_argument("--method", choices=["ai", "nearest", "lanczos"],
                        default="nearest",
                        help="Upscaling method: ai (Real-ESRGAN), nearest (default), lanczos")
    parser.add_argument("--deploy", metavar="GAME_DIR",
                        help="Copy upscaled output into GAME_DIR/hires/ after upscaling")
    args = parser.parse_args()

    extracted = Path(args.extracted_dir)
    hires     = Path(args.hires_dir)
    force_method = args.method

    if not extracted.is_dir():
        sys.exit(f"Extracted assets directory not found: {extracted}\n"
                 f"Run extract_assets.py or extract_scenes.py first.")

    # Collect all leaf directories that contain PNGs
    src_dirs = [d for d in sorted(extracted.rglob("*"))
                if d.is_dir() and list(d.glob("*.png"))]
    if not src_dirs:
        sys.exit(f"No PNG files found under {extracted}")

    # Determine upscaling method
    use_esrgan = False
    pil_resample = Image.NEAREST
    binary = None

    if force_method == "ai":
        binary = ensure_esrgan()
        if binary:
            model_dir = binary.parent / "models"
            if not model_dir.is_dir():
                print(f"  Model dir not found at {model_dir}, checking parent...")
                model_dir = binary.parent.parent / "models"
            if model_dir.is_dir():
                test_src = src_dirs[0]
                test_dst = hires / test_src.relative_to(extracted)
                print(f"Testing Real-ESRGAN on {test_src.name}...")
                use_esrgan = upscale_folder_esrgan(binary, test_src, test_dst, model_dir)
            else:
                print(f"  Model directory not found ({model_dir}) — falling back to PIL nearest")
        if not use_esrgan:
            print("Real-ESRGAN unavailable — falling back to PIL nearest 4×\n")
    elif force_method == "lanczos":
        pil_resample = Image.LANCZOS
        print(f"Using PIL Lanczos {SCALE}×\n")
    else:  # nearest (default)
        pil_resample = Image.NEAREST
        print(f"Using PIL nearest {SCALE}×\n")

    if use_esrgan:
        print(f"Using Real-ESRGAN ({MODEL}, {SCALE}×)\n")

    # Process all directories
    total_files = 0
    for src_dir in src_dirs:
        rel      = src_dir.relative_to(extracted)
        dst_dir  = hires / rel
        n_pngs   = len(list(src_dir.glob("*.png")))

        print(f"  {rel} ({n_pngs} images)...", end=" ", flush=True)

        if use_esrgan:
            ok = upscale_folder_esrgan(binary, src_dir, dst_dir, model_dir)
            if not ok:
                print("esrgan failed → PIL fallback", end=" ", flush=True)
                upscale_folder_pil(src_dir, dst_dir, pil_resample)
        else:
            upscale_folder_pil(src_dir, dst_dir, pil_resample)

        done = len(list(dst_dir.glob("*.png")))
        print(f"→ {done} done")
        total_files += done

    if use_esrgan:
        method = f"Real-ESRGAN {MODEL} {SCALE}×"
    elif pil_resample == Image.LANCZOS:
        method = f"PIL Lanczos {SCALE}×"
    else:
        method = f"PIL nearest {SCALE}×"
    print(f"\nDone. {total_files} images upscaled ({method})")
    print(f"Output: {hires}/")

    # ── Optional deploy ────────────────────────────────────────────────────────
    if args.deploy:
        import shutil
        game_dir = Path(args.deploy)
        if not game_dir.is_dir():
            print(f"\nWarning: game directory not found: {game_dir} — skipping deploy")
        else:
            # hires_dir="hires"        → copy into GAME_DIR/hires/
            # hires_dir="hires/scenes" → copy into GAME_DIR/hires/scenes/
            if hires.name == "hires":
                dest = game_dir / "hires"
            else:
                dest = game_dir / "hires" / hires.name
            print(f"\nDeploying {hires}/ → {dest}/")
            shutil.copytree(str(hires), str(dest), dirs_exist_ok=True)
            print("Deploy done.")
    else:
        print()
        print("Next steps:")
        print("  1. Rebuild OpenJazz with -DHIRES=ON")
        print(f"  2. Copy {hires}/ into your game data directory's hires/ folder")
        print("  3. Run OpenJazz — hi-res assets will be loaded automatically")


if __name__ == "__main__":
    main()
