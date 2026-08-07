#!/usr/bin/env python3
"""Render the images the installer wizard needs, from the AbstractumVPN mark.

Companion to generate_icons.py, and it borrows that file's geometry: the mark is
three bars with the middle one displaced, drawn with Pillow rather than
rasterising the SVG. Keep the two in step - if the mark changes there, it
changes here.

Run after any change to the mark or the palette:

    python deploy/generate_installer_assets.py

Everything lands in deploy/installer/qif/images/ and is wired up in
cmake/CPack.cmake. PNGs are generated rather than committed by hand so that a
change to the mark cannot leave the installer showing the old one.

Requires only Pillow.
"""

import os
import sys

try:
    from PIL import Image, ImageDraw
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join("deploy", "installer", "qif", "images")

# Palette, mirrored from client/ui/qml/Modules/Style/AmneziaStyle.qml. The
# stylesheet next door repeats these values as hex; change both together.
STEEL = (155, 191, 222, 255)      # #9BBFDE
STEEL_MUTED = (62, 93, 117, 255)  # #3E5D75
CRT = (10, 10, 10, 255)           # #0A0A0A, the wizard background
LINE = (35, 42, 49, 255)          # #232A31, the divider colour

# Same 64-unit space as generate_icons.py.
BARS = [
    (6, 16, 40, 8, STEEL),
    (18, 28, 40, 8, STEEL_MUTED),
    (6, 40, 40, 8, STEEL),
]
RADIUS = 4
SS = 4  # supersampling factor


def write(img, name):
    path = os.path.join(ROOT, OUT, name)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.save(path)
    print(f"  {img.size[0]:>4}x{img.size[1]:<4}  {os.path.join(OUT, name)}")


def draw_mark(draw, ox, oy, size):
    """Paint the mark at (ox, oy) occupying `size` px, in supersampled space."""
    scale = size / 64.0
    for x, y, w, h, colour in BARS:
        draw.rounded_rectangle(
            [ox + x * scale, oy + y * scale,
             ox + (x + w) * scale, oy + (y + h) * scale],
            radius=RADIUS * scale,
            fill=colour,
        )


def render_window_icon(size):
    """QWizard window icon. Transparent so it sits on any title bar."""
    big = size * SS
    img = Image.new("RGBA", (big, big), (0, 0, 0, 0))
    draw_mark(ImageDraw.Draw(img), 0, 0, big)
    return img.resize((size, size), Image.LANCZOS)


def render_logo(size=48):
    """QWizard::LogoPixmap - the small mark in the page header."""
    return render_window_icon(size)


def render_banner(width=600, height=48):
    """QWizard::BannerPixmap - the strip across the top in the Modern style.

    Mark on the left, then a hairline rule running to the right edge. The rule
    is what makes the header read as a terminal frame rather than as a logo
    sitting on empty space.
    """
    big_w, big_h = width * SS, height * SS
    img = Image.new("RGBA", (big_w, big_h), CRT)
    draw = ImageDraw.Draw(img)

    mark = int(big_h * 0.58)
    pad = (big_h - mark) // 2
    draw_mark(draw, pad, pad, mark)

    # Rule at the vertical centre, starting clear of the mark.
    rule_y = big_h // 2
    draw.rectangle(
        [pad * 2 + mark, rule_y - SS // 2, big_w - pad, rule_y + SS // 2],
        fill=LINE,
    )

    # Tick marks along the rule, sparse, fading out. Purely decorative - they
    # give the strip the look of a measuring scale.
    step = big_w // 24
    for i in range(1, 24):
        x = pad * 2 + mark + i * step
        if x > big_w - pad:
            break
        draw.rectangle(
            [x, rule_y - SS * 2, x + SS // 2, rule_y + SS * 2],
            fill=STEEL_MUTED if i % 4 == 0 else LINE,
        )

    return img.resize((width, height), Image.LANCZOS)


def render_scanline():
    """A 1x4 tile: three clear rows and one barely-lit row.

    Tiled by the stylesheet to fake a CRT sweep, because Qt Style Sheets have no
    repeating gradients. Kept extremely faint on purpose - at full strength it
    turns into moire on high-DPI screens.

    Not supersampled: this must stay pixel-exact or the tiling seams show.
    """
    img = Image.new("RGBA", (1, 4), (0, 0, 0, 0))
    img.putpixel((0, 0), (255, 255, 255, 6))
    return img


def main():
    print("Installer assets:")
    write(render_banner(), "banner.png")
    write(render_logo(), "logo.png")
    write(render_window_icon(64), "window_icon.png")
    write(render_scanline(), "scanline.png")


if __name__ == "__main__":
    main()
