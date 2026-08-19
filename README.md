# cutie-gallery

A minimal gallery app for Cutie Shell: date-sorted grid with two-stage pinch
zoom, and a swipe-through full-screen viewer with pinch-to-zoom and
double-tap-to-reset.

# Disclaimer 

This code is written to be as simple as possible by taking advantage of as many pre-made libraries as possible. Moreover, this code was written with the help of AI. If you are interested in this codebase, please review it and submit pull requests. Thank you for your attention regarding this matter.

## Build

```
cmake -B build -S .
cmake --build build
./build/cutie-gallery
```

Requires Qt6 (built and tested against 6.4.2): Core, Gui, Qml, Quick,
Concurrent, plus the QtQuick Controls/Templates/WorkerScript QML runtime
modules.

I compiled this and ran it headlessly (`QT_QPA_PLATFORM=offscreen`) against
a fake `$HOME` with dummy JPEGs to confirm it builds clean and the
scan/dedupe/sort/thumbnail-decode pipeline actually works end to end. I have
not tested it on-device (touch, hwcomposer, real photos), so treat gesture
feel as a starting point, not final.

## What's here

- `src/imagescanner.*` — background-thread scan of `~/Pictures`, `~/DCIM`,
  `~/DCIM/Camera` (non-recursive), deduped by canonical path, exposed as a
  flat `QAbstractListModel` sorted newest-first.
- `src/thumbnailprovider.*` — async `image://cutiegallerythumb/...` provider,
  decodes off the UI thread via `QThreadPool`, scales
  `KeepAspectRatioByExpanding` so `PreserveAspectCrop` thumbnails stay sharp.
  In-memory only, no disk cache — simplest thing that works.
- `qml/GalleryGridPage.qml` — grid with two fixed zoom levels (3 / 5
  columns), toggled by pinch.
- `qml/ImageViewerPage.qml` — horizontal `ListView` (virtualizing, so it
  won't choke on hundreds of photos the way `SwipeView`+`Repeater` would),
  pinch-to-zoom per photo via `PinchArea`, double-tap to reset, swiping
  disabled while zoomed in.
- `qml/main.qml` — plain `StackView` for navigation.

## One real finding worth knowing

I originally had `GridView.section` for "Today / Yesterday / August 15,
2026" style headers. Turns out `GridView` doesn't support `section` at all
in Qt6 — that's `ListView`-only, and I only found this by actually running
it (`Cannot assign to non-existent property "section"`). I dropped the
header row rather than ship something silently broken. The sort order
itself is unaffected — photos are still newest-first. If you want the
headers back, the clean way is a mixed model (header rows and photo rows
interleaved, `Loader`-based delegate to switch layout) rather than GridView.

## Known simplifications / things to tune on-device

- **Styling is placeholder.** Toolbar, back button, section colors are
  generic Qt Quick Controls — swap in your Cutie theme components
  (`ListItem.qml` conventions, etc.) for visual consistency.
- **`pageStack` assumed to be a plain `StackView`.** If `qml-module-cutie`
  has a shared pageStack with a different `push()` signature, swap it in —
  it's isolated to `main.qml`.
- **Pinch bounds in the viewer are loose**, not tightly clamped to the
  actual scaled-image edges (`pinch.minimumX`/`maximumX` etc. use a fixed
  multiple of image size rather than a per-scale-level exact bound). Works,
  but allows some over-pan past the edge — worth tightening once you can
  feel it on a touchscreen.
- **No thumbnail disk cache**, so thumbnails redecode if scrolled far enough
  away and back. Fine for a first pass; add a cache keyed on
  `path + mtime` under `QStandardPaths::CacheLocation` if it's ever the
  bottleneck.
