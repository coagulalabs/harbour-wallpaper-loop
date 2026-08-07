# Wallpaper Loop — harbour-wallpaper-loop

Native Sailfish OS app that cycles through a folder of images and applies each one as your **Ambience** wallpaper on a timer.

Sailfish port of the Android app [`dev.wallpaper.loop`](https://github.com/coagulalabs) (Wallpaper Loop).

## Features

- Folder browser (Pictures, Downloads, Home)
- Interval slider + presets (30s–1 month)
- Background **systemd user service** — close the UI freely
- Sequential or shuffle order
- Fill / Fit / Contain scaling
- Include subfolders
- Next / Previous from the app and cover
- Settings persist across launches

## Requirements

- Sailfish OS 5.x (aarch64 or armv7hl)
- Sailjail permissions: Ambience, AppLaunch, UserDirs, Downloads, Pictures

## Build & install

From the SailfishOS workspace root:

```bash
./scripts/deploy-wallpaper-loop.sh          # build + install on phone
./scripts/deploy-wallpaper-loop.sh --clean  # clean rebuild
./scripts/package-wallpaper-loop.sh all     # aarch64 + armv7hl → dist-wallpaper-loop/
```

Requires Sailfish SDK targets `SailfishOS-5.1.0.11-aarch64` and `…-armv7hl` (or compatible 5.0+).

## Background service

Turning **Slideshow** on starts `harbour-wallpaper-loop.service` in your user session (`harbour-wallpaper-loop --daemon`). It shares settings with the UI and calls `setAmbience` on the interval. Turning Slideshow off stops and disables the unit.

```bash
systemctl --user status harbour-wallpaper-loop.service
```

## Publish (GitHub + OpenRepos)

```bash
./scripts/publish-wallpaper-loop.sh                           # build both arches + cache RPMs
./scripts/publish-wallpaper-loop.sh --github                  # + GitHub release
OPENREPOS_USERNAME=... OPENREPOS_PASSWORD=... \
  ./scripts/publish-wallpaper-loop.sh --openrepos             # + OpenRepos upload
```

OpenRepos listing copy: [`openrepos/listing.md`](openrepos/listing.md)

Cached RPMs:
- `dist-wallpaper-loop/harbour-wallpaper-loop-1.5.0-1.aarch64.rpm`
- `dist-wallpaper-loop/harbour-wallpaper-loop-1.5.0-1.armv7hl.rpm`

## How wallpaper works on Sailfish

Home/lock visuals are **Ambiences**, managed by `ambienced`. This app calls:

```
com.jolla.ambienced.setAmbience(file:///path/to/image.jpg)
```

Home and lock share the same Ambience image.

## Limitations (1.5.0)

- Still images only (JPG, PNG, WebP, BMP, GIF first frame)
- No separate home vs lock targets
- No Android-style live / video wallpaper under the home grid

## License

BSD-3-Clause — Copyright © 2026 Coagula / coagulalabs
