#!/usr/bin/env python3
"""Render every application icon from the AbstractumVPN mark.

The mark is three bars, the middle one displaced - see
client/images/AbstractumVPN_logo.svg, which is the source of truth for the
geometry. This script reproduces that geometry with Pillow rather than
rasterising the SVG, because the shape is three rounded rectangles and adding a
rasteriser dependency to build a shape this simple is not worth it.

Run after any change to the mark:

    python deploy/generate_icons.py

Requires only Pillow. Everything is drawn at 4x and downsampled, which gives
clean edges without hinting.
"""

import os
import sys

try:
    from PIL import Image, ImageDraw
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Palette, mirrored from client/ui/qml/Modules/Style/AmneziaStyle.qml.
STEEL = (155, 191, 222, 255)   # #9BBFDE
STEEL_MUTED = (62, 93, 117, 255)  # #3E5D75
ONYX = (28, 29, 33, 255)       # #1C1D21, used where alpha is not allowed

# Geometry in the 64-unit space of the SVG: (x, y, w, h, colour).
BARS = [
    (6, 16, 40, 8, STEEL),
    (18, 28, 40, 8, STEEL_MUTED),
    (6, 40, 40, 8, STEEL),
]
RADIUS = 4
SS = 4  # supersampling factor


def render(size, background=None):
    """Draw the mark at `size` px. Opaque `background` disables alpha."""
    big = size * SS
    scale = big / 64.0
    mode = "RGBA"
    base = background if background else (0, 0, 0, 0)
    img = Image.new(mode, (big, big), base)
    draw = ImageDraw.Draw(img)

    for x, y, w, h, colour in BARS:
        draw.rounded_rectangle(
            [x * scale, y * scale, (x + w) * scale, (y + h) * scale],
            radius=RADIUS * scale,
            fill=colour,
        )

    img = img.resize((size, size), Image.LANCZOS)
    if background:
        img = img.convert("RGB")  # iOS rejects an alpha channel
    return img


def write(img, *parts):
    path = os.path.join(ROOT, *parts)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.save(path)
    print(f"  {img.size[0]:>4}x{img.size[1]:<4}  {os.path.join(*parts)}")


# --- tray icons -------------------------------------------------------------
# A separate pass because these encode connection state, not brand. Sizes match
# what the app already ships (200 px; Qt downscales at runtime).
#
# R3 in the roadmap adds a fourth state - up but no traffic. The mark already
# has a form for it: hollow middle bar, see render_tray(hollow_middle=True).
# It is not wired up because the state does not exist in the code yet.

TRAY_STATES = {
    "active": STEEL,                     # connected
    "default": (135, 139, 145, 255),     # #878B91, disconnected
    "error": (235, 87, 87, 255),         # #EB5757
}


def render_tray(size, colour, hollow_middle=False):
    big = size * SS
    scale = big / 64.0
    img = Image.new("RGBA", (big, big), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    for i, (x, y, w, h, _) in enumerate(BARS):
        box = [x * scale, y * scale, (x + w) * scale, (y + h) * scale]
        if hollow_middle and i == 1:
            draw.rounded_rectangle(box, radius=RADIUS * scale,
                                   outline=colour, width=int(2 * scale))
        else:
            draw.rounded_rectangle(box, radius=RADIUS * scale, fill=colour)
    return img.resize((size, size), Image.LANCZOS)


def generate_tray():
    print("Tray")
    for name, colour in TRAY_STATES.items():
        write(render_tray(200, colour), "client", "images", "tray", f"{name}.png")


def render_wide(width, height):
    """Mark centred on a transparent canvas of arbitrary proportions.

    Used for the large in-app logo. Deliberately mark-only: rendering a wordmark
    here would bake a typeface into a redistributed asset, and no font is
    licensed for the project yet. The lock-up with text lives in
    client/images/AbstractumVPN_Full_logo.svg, which stays vector.
    """
    big_w, big_h = width * SS, height * SS
    img = Image.new("RGBA", (big_w, big_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Fit the 64-unit square into the canvas with a 12% margin, then centre it.
    side = min(big_w, big_h) * 0.76
    scale = side / 64.0
    off_x = (big_w - 64 * scale) / 2
    off_y = (big_h - 64 * scale) / 2

    for x, y, w, h, colour in BARS:
        draw.rounded_rectangle(
            [off_x + x * scale, off_y + y * scale,
             off_x + (x + w) * scale, off_y + (y + h) * scale],
            radius=RADIUS * scale, fill=colour,
        )
    return img.resize((width, height), Image.LANCZOS)


def main():
    print("Application icons")
    write(render(256), "client", "images", "icon.png")
    write(render(256), "client", "images", "AbstractumVPN.png")
    write(render_wide(1440, 1200), "client", "images", "AbstractumBigLogo.png")
    write(render(512), "deploy", "data", "linux", "AbstractumVPN.png")
    write(render(480), "metadata", "en-US", "images", "icon.png")

    print("Windows .ico")
    ico_sizes = [16, 24, 32, 48, 64, 128, 256]
    largest = render(256)
    path = os.path.join(ROOT, "client", "images", "app.ico")
    largest.save(path, sizes=[(s, s) for s in ico_sizes])
    print(f"  {','.join(str(s) for s in ico_sizes)}  client/images/app.ico")

    # iOS icons must be opaque - the App Store rejects an alpha channel.
    print("iOS (opaque)")
    ios = [20, 29, 40, 50, 57, 58, 60, 72, 76, 80, 87,
           100, 114, 120, 144, 152, 167, 180, 1024]
    for s in ios:
        write(render(s, background=ONYX),
              "client", "ios", "app", "Media.xcassets",
              "AppIcon.appiconset", f"{s}.png")

    generate_tray()

    print("macOS")
    mac = [(16, "16"), (32, "16@2x"), (32, "32"), (64, "32@2x"),
           (64, "64"), (128, "64@2x"), (128, "128"), (256, "128@2x"),
           (256, "256"), (512, "256@2x"), (512, "512"), (1024, "512@2x")]
    for catalogue in ("Images.xcassets", "Images-beta.xcassets"):
        target = os.path.join(ROOT, "client", "macos", "app", catalogue,
                              "AppIcon.appiconset")
        if not os.path.isdir(target):
            continue
        for size, name in mac:
            if not os.path.exists(os.path.join(target, f"{name}.png")):
                continue  # only replace icons the catalogue already declares
            write(render(size), "client", "macos", "app", catalogue,
                  "AppIcon.appiconset", f"{name}.png")

    print("\nNot generated here:")
    print("  client/images/app.icns - Pillow cannot write icns.")
    print("    Build it on macOS from the AppIcon.appiconset above:")
    print("    iconutil -c icns AppIcon.appiconset -o app.icns")


if __name__ == "__main__":
    main()
