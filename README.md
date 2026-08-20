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

Requires Qt6 (Core, Quick, Qml, Concurrent) plus the real `qml6-module-cutie`
package for the `Cutie`/`Atmosphere` imports to resolve - I don't have that
module in my sandbox, so unlike the first pass at this project, **I could
not run this one end-to-end.**

What I could and did verify: the C++ compiles and links cleanly against
plain Qt6 (the `Cutie` QML module isn't needed at C++ compile time, only
when the QML actually loads), and running the built binary gets exactly as
far as it can without that module - it fails on the single expected line,
`module "Cutie" is not installed`, with nothing else. The QtQuick-level
mechanics (GridView, ListView, PinchArea, TapHandler, Behavior, anchors) are
otherwise unchanged from the version I did fully build and run earlier, so
I'm confident in those. The Cutie-layer QML (`CutieWindow`, `CutiePage`,
`CutiePageHeader`, `CutieButton`, `CutieLabel`, `Atmosphere.*`) is written to
match what's actually used in `cutie-notes`, not guessed at - but I haven't
seen it run.

## What changed from the plain-QtQuick-Controls version

Restructured to mirror `cutie-notes`' actual layout and conventions:

- **Directory layout**: `src/main.cpp`, `src/imagescanner.*`,
  `src/thumbnailprovider.*`, `src/qml/*.qml` with a flat `qml.qrc` - same
  shape as `cutie-notes`, not the `qml/` + top-level `src/` split I used the
  first time.
- **`ImageScanner` registered as a real QML module singleton**
  (`qmlRegisterSingletonType<ImageScanner>("CutieGallery", 1, 0,
  "ImageScanner", ...)`, imported as `import CutieGallery`), the same way
  `NotesManager` is registered under `"CutieNotes"`, instead of a context
  property.
- **Real Cutie components**: `CutieWindow` (with its own `pageStack` and
  `initialPage`), `CutiePage`, `CutiePageHeader`, `CutieLabel`, `CutieButton`,
  and `Atmosphere.textColor` / `Atmosphere.secondaryAlphaColor` for
  theming - all taken directly from how `cutie-notes` actually uses them,
  not invented.
- **Grid page inlined into `main.qml`**, the same way `cutie-notes`' note
  grid is inlined rather than split into its own file - only the pushed
  detail page (`ImageViewerPage.qml`, paralleling `NoteView.qml`) is
  separate.
- **Photo cards match the note-card treatment**: a separate rounded
  background rectangle (`Atmosphere.secondaryAlphaColor` at 0.15 opacity,
  radius 12) behind each thumbnail, same as the note cards, with the image
  inset slightly further so its square corners sit inside the rounded frame.
- **Refresh FAB** in the same visual language as the new-note FAB
  (transparent fill, thin `Atmosphere.textColor` ring, centered glyph),
  wired to `ImageScanner.refresh()`.
- **Tabs, `Q_SIGNALS`/`Q_EMIT`, `Q_INVOKABLE` without `slots:` blocks** in
  the C++ - matching `notesmanager.h/cpp` exactly rather than the
  spaces/`signals`/`emit` style from the first pass.
- **`debian/`, `.desktop`, `.gitignore`, icon** all brought in line with
  `cutie-notes`' actual packaging (same build-deps, same `xdg-open` desktop
  entry shape, same icon style adapted to a gallery glyph).

## One assumption I couldn't verify

`NoteView.qml` doesn't add its own back button, which suggests
`CutiePageHeader` may auto-provide one once pushed onto `pageStack` - but I
have no way to confirm that without the actual component source. Given "a
back arrow at the top" was an explicit requirement, I kept an explicit
`CutieButton` with a `go-previous-symbolic` icon in `ImageViewerPage.qml`'s
header rather than assume it's redundant. If `CutiePageHeader` does provide
one automatically, you'll see two - just delete the explicit one.

## Everything else from the previous README still applies

Loose pinch-zoom bounds in the viewer, no thumbnail disk cache, and the
`GridView.section` finding (it doesn't exist in Qt6 - `ListView`-only) are
all unchanged. See the git history / previous version for that detail if
you don't already have it.
