#!/usr/bin/env python3
"""Host unit tests mirroring src/imageutil.h (no Qt required)."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path


def is_image_file(name: str) -> bool:
    lower = name.lower()
    return any(
        lower.endswith(ext)
        for ext in (".jpg", ".jpeg", ".png", ".webp", ".bmp", ".gif")
    )


def clamp_interval_seconds(seconds: int) -> int:
    return max(15, min(2592000, seconds))  # max ~30 days


DAY = 86400
WEEK = 604800
MONTH = 2592000


def normalize_order(order: str) -> str:
    return "SHUFFLE" if order == "SHUFFLE" else "SEQUENTIAL"


def normalize_scale_mode(mode: str) -> str:
    if mode in ("FIT", "CONTAIN"):
        return mode
    return "FILL"


def next_index(current: int, count: int) -> int:
    if count <= 0:
        return 0
    return (current + 1) % count


def previous_index(current: int, count: int) -> int:
    if count <= 0:
        return 0
    return (current - 1 + count) % count


IMAGE_FILTERS = (
    "*.jpg",
    "*.jpeg",
    "*.png",
    "*.webp",
    "*.bmp",
    "*.gif",
    "*.JPG",
    "*.JPEG",
    "*.PNG",
    "*.WEBP",
    "*.BMP",
    "*.GIF",
)


class ImageUtilTests(unittest.TestCase):
    def test_is_image_file(self):
        self.assertTrue(is_image_file("a.jpg"))
        self.assertTrue(is_image_file("a.JPEG"))
        self.assertTrue(is_image_file("shot.PNG"))
        self.assertTrue(is_image_file("x.webp"))
        self.assertTrue(is_image_file("x.bmp"))
        self.assertTrue(is_image_file("anim.gif"))
        self.assertFalse(is_image_file("clip.mp4"))
        self.assertFalse(is_image_file("clip.webm"))
        self.assertFalse(is_image_file("readme.txt"))
        self.assertFalse(is_image_file("wallpaper"))

    def test_clamp_interval(self):
        self.assertEqual(clamp_interval_seconds(1), 15)
        self.assertEqual(clamp_interval_seconds(15), 15)
        self.assertEqual(clamp_interval_seconds(300), 300)
        self.assertEqual(clamp_interval_seconds(DAY), DAY)
        self.assertEqual(clamp_interval_seconds(WEEK), WEEK)
        self.assertEqual(clamp_interval_seconds(MONTH), MONTH)
        self.assertEqual(clamp_interval_seconds(9999999), MONTH)

    def test_normalize_order(self):
        self.assertEqual(normalize_order("SEQUENTIAL"), "SEQUENTIAL")
        self.assertEqual(normalize_order("SHUFFLE"), "SHUFFLE")
        self.assertEqual(normalize_order("random"), "SEQUENTIAL")

    def test_normalize_scale(self):
        self.assertEqual(normalize_scale_mode("FILL"), "FILL")
        self.assertEqual(normalize_scale_mode("FIT"), "FIT")
        self.assertEqual(normalize_scale_mode("CONTAIN"), "CONTAIN")
        self.assertEqual(normalize_scale_mode("stretch"), "FILL")

    def test_next_previous_wrap(self):
        self.assertEqual(next_index(0, 0), 0)
        self.assertEqual(previous_index(0, 0), 0)
        self.assertEqual(next_index(2, 3), 0)
        self.assertEqual(next_index(0, 3), 1)
        self.assertEqual(previous_index(0, 3), 2)
        self.assertEqual(previous_index(1, 3), 0)

    def test_scan_folder_filters(self):
        with tempfile.TemporaryDirectory() as root:
            Path(root, "note.txt").write_text("x")
            Path(root, "photo.JPG").write_text("x")
            Path(root, "clip.mp4").write_text("x")
            Path(root, "anim.gif").write_text("x")
            kept = sorted(
                p.name.lower()
                for p in Path(root).iterdir()
                if p.is_file() and is_image_file(p.name)
            )
            self.assertEqual(kept, ["anim.gif", "photo.jpg"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
