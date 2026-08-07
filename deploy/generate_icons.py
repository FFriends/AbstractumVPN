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

# Backing plate for the application icon. The bare mark is three thin bars on
# transparency, and a shortcut has to survive whatever the user set as their
# wallpaper - on a light or busy one the mark washed out. Everything below is in
# the same 64-unit space as the bars.
#
# Not used by the tray icons: those are state indicators sitting on the system's
# own panel, and a plate there would read as a second, competing icon.
PLATE = (28, 29, 33, 255)           # #1C1D21 onyx, same as the iOS background
PLATE_EDGE = (62, 93, 117, 255)     # #3E5D75, keeps the plate off a dark wallpaper
PLATE_RADIUS = 14                   # ~22% of the side: a rounded square, not a circle
PLATE_EDGE_WIDTH = 1
MARK_SCALE = 0.74                   # the mark shrinks to leave a margin inside the plate

# Below this the margin costs more than it buys: at 16 px a mark at 0.74 leaves
# bars a pixel and a half tall, and the middle one merges into the top one. Small
# sizes get more of the plate and no edge - a one-pixel outline there eats a
# sixteenth of the icon.
SMALL_SIZE = 24
SMALL_MARK_SCALE = 0.88


def render(size, background=None, plate=True):
    """Draw the mark at `size` px.

    `plate` puts the mark on a rounded square. `background` fills the whole
    canvas opaquely and is meant for iOS, which rejects an alpha channel and
    applies its own rounded mask - drawing our own plate under that mask would
    show a rounded square inside a rounded square.
    """
    big = size * SS
    scale = big / 64.0
    mode = "RGBA"
    base = background if background else (0, 0, 0, 0)
    img = Image.new(mode, (big, big), base)
    draw = ImageDraw.Draw(img)

    # The mark keeps its proportions and moves to the centre of the plate.
    small = size < SMALL_SIZE
    if plate:
        mark = SMALL_MARK_SCALE if small else MARK_SCALE
    else:
        mark = 1.0
    offset = 32.0 * (1.0 - mark)

    if plate:
        if small:
            draw.rounded_rectangle(
                [0, 0, big - 1, big - 1],
                radius=PLATE_RADIUS * scale,
                fill=PLATE,
            )
        else:
            inset = PLATE_EDGE_WIDTH * scale / 2
            draw.rounded_rectangle(
                [inset, inset, big - 1 - inset, big - 1 - inset],
                radius=PLATE_RADIUS * scale,
                fill=PLATE,
                outline=PLATE_EDGE,
                width=max(1, int(round(PLATE_EDGE_WIDTH * scale))),
            )

    for x, y, w, h, colour in BARS:
        draw.rounded_rectangle(
            [(offset + x * mark) * scale, (offset + y * mark) * scale,
             (offset + (x + w) * mark) * scale, (offset + (y + h) * mark) * scale],
            radius=RADIUS * mark * scale,
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
        write(render(s, background=ONYX, plate=False),
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
