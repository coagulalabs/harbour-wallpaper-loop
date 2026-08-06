# OpenRepos listing — Wallpaper Loop

Use these fields when creating the application on [openrepos.net](https://openrepos.net).

## Application name

Wallpaper Loop

## Package / project name

`harbour-wallpaper-loop`

## Category

System

## Summary (short)

Cycle folder images as your Sailfish Ambience wallpaper.

## Description (body)

**Wallpaper Loop** is a native Sailfish OS app that cycles through images in a folder and applies each one as your Ambience wallpaper on a timer.

Sailfish port of the Android app `dev.wallpaper.loop`.

### Features

- Pick any folder (Pictures, Downloads, Home, including Android storage Downloads)
- Interval slider with presets from 30 seconds to 1 hour
- Sequential or shuffle playback
- Fill / Fit / Contain scaling
- Optional subfolder scanning
- Next / Previous from the app and from the cover
- Settings saved between launches

### How it works

Sailfish does not use Android-style live wallpapers. This app calls the Ambience daemon (`com.jolla.ambienced.setAmbience`) so each image becomes the active Ambience. Home and lock screens share the same wallpaper.

### Requirements

- Sailfish OS 5.x (aarch64)
- Ambience, Pictures, Downloads, and UserDirs permissions (requested via Sailjail)

### Limitations (1.3.0)

- Still images only (JPG, PNG, WebP, BMP, GIF first frame)
- Slideshow timer runs while the app process is alive (app open or covered)
- No separate home vs lock targets (Ambience is shared)
- No Android-style live / video wallpaper under the home grid

### Changelog

**1.3.0** — Restore stills-only Ambience slideshow (no video, no live layer, no preview).

**1.0.0** — Initial OpenRepos release: folder slideshow via setAmbience, intervals, shuffle/sequential, scaling, cover actions.

### Install

Via Storeman / OpenRepos client, or:

```
devel-su pkcon install-local harbour-wallpaper-loop-1.3.0-1.aarch64.rpm
```

RPM path after packaging: `dist-wallpaper-loop/harbour-wallpaper-loop-1.3.0-1.aarch64.rpm`

Paste-ready plain text for the OpenRepos form: [`description.txt`](description.txt)

### Links

- Source: https://github.com/coagulalabs/harbour-wallpaper-loop
- Android original package: `dev.wallpaper.loop`

### License

BSD-3-Clause
