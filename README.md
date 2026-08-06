# Wallpaper Loop — harbour-wallpaper-loop

Native Sailfish OS app that cycles through a folder of images and applies each one as your **Ambience** wallpaper on a timer.

Sailfish port of the Android app [`dev.wallpaper.loop`](https://github.com/coagulalabs) (Wallpaper Loop).

## Features

- Folder browser (Pictures, Downloads, Home)
- Interval slider + presets (30s–1h)
- Sequential or shuffle order
- Fill / Fit / Contain scaling
- Include subfolders
- Next / Previous from the app and cover
- Settings persist across launches

## Requirements

- Sailfish OS 5.x (aarch64)
- Sailjail permissions: Ambience, UserDirs, Downloads, Pictures

## Build & install

From the SailfishOS workspace root:

```bash
./scripts/deploy-wallpaper-loop.sh          # build + install on phone
./scripts/deploy-wallpaper-loop.sh --clean  # clean rebuild
./scripts/package-wallpaper-loop.sh         # RPM → dist-wallpaper-loop/
```

Requires Sailfish SDK target `SailfishOS-5.1.0.11-aarch64` (or compatible 5.0+).

## Publish (GitHub + OpenRepos)

```bash
./scripts/publish-wallpaper-loop.sh                           # build + cache RPM
./scripts/publish-wallpaper-loop.sh --github                  # + GitHub release
OPENREPOS_USERNAME=... OPENREPOS_PASSWORD=... \
  ./scripts/publish-wallpaper-loop.sh --openrepos             # + OpenRepos upload
```

OpenRepos listing copy: [`openrepos/listing.md`](openrepos/listing.md)

Cached RPM: `dist-wallpaper-loop/harbour-wallpaper-loop-1.3.0-1.aarch64.rpm`

## How wallpaper works on Sailfish

Sailfish does not expose Android’s `WallpaperManager`. Home/lock visuals are **Ambiences**, managed by `ambienced`. This app calls:

```
com.jolla.ambienced.setAmbience(file:///path/to/image.jpg)
```

Home and lock share the same Ambience image.

## Limitations (1.3.0)

- Still images only (JPG, PNG, WebP, BMP, GIF first frame)
- Timer runs while the app process is alive
- No separate home vs lock targets

## License

BSD-3-Clause — Copyright © 2026 Coagula / coagulalabs
