# Okinawa documentation site — design

**Date:** 2026-06-18
**Repo:** `okinawa-dev/okinawa.cpp` (branch `master`)
**Status:** approved design, pending implementation plan

## Goal

A documentation website for the **Okinawa engine**, authored in Markdown,
built on every push to `master` and deployed automatically to GitHub Pages
at `https://okinawa-dev.github.io/okinawa.cpp/`.

Two pillars:

1. **API reference** — curated by hand, one page per public subsystem.
2. **Examples / tutorials** — the strong, didactic section. First two
   tutorials: (a) creating `OkItem`s on the fly and relating them to each
   other (attach + per-frame rotate), and (b) building a texture viewer.

The approach mirrors the sibling repos `nottario` and `owl` (Markdown
content → custom static generator → GitHub Actions → Pages), but **fits
okinawa conceptually**: the generator is a small **C++ tool built with
xmake**, not the Go tool those repos use. The visual design is authored
from scratch with the `impeccable` skill — we do **not** copy the
GitHub-like theme of the other two sites.

## Non-goals

- No Go toolchain.
- No Doxygen / auto-extracted API reference (curated by hand instead; can
  be added later if the API grows).
- No YAML library and no third-party template engine (kept minimal on
  purpose — see the generator section).
- The generator does **not** link the engine (no OpenGL/GLFW in CI).

## Repository layout (new, all under `docs/`)

```
docs/
  content/                 # Markdown sources (the only thing authors edit)
    index.md
    getting-started.md
    reference/             # API reference, curated by hand
      core.md              # OkCore, OkCamera, OkObject
      items.md             # OkItem, OkItemGroup, OkTexture
      scene.md             # OkScene
      math.md              # OkPoint, OkRotation, OkMath
      input.md             # OkInput, keys
      handlers.md          # OkSceneHandler, OkTextureHandler
      config.md            # OkConfig
      mcp.md               # OkMcpServer
    examples/
      objects-on-the-fly.md   # create OkItems at runtime + attach/rotate
      texture-viewer.md       # build a texture-preview viewer
  templates/
    layout.html            # single page template (designed by impeccable)
  static/
    docs.css               # site styles (designed by impeccable)
    okinawa_logo.png       # copied from assets/project/
    favicon.*              # as the impeccable design dictates
  tool/
    main.cpp               # the C++ generator
    xmake.lua (optional)   # or a target in the root xmake.lua — see below
  dist/                    # generated HTML output — gitignored
```

`docs/dist/` is added to `.gitignore`.

## The generator (`docs/tool`)

A standalone C++17 program, built with the same xmake/clang the engine
uses, so okinawa documents okinawa with its own tooling.

### Build integration

- New xmake target **`okinawa-docs`** (binary) declared with
  **`set_default(false)`** so a plain `xmake` (engine build) skips it. It
  is built explicitly with `xmake build okinawa-docs`.
- The target does **not** `add_deps("okinawa")` — it has nothing to do
  with the renderer. Its only third-party dependency is **`md4c`** via
  xrepo (`add_requires("md4c")` / `add_packages("md4c")`).
- Decision to confirm during implementation: declare the target in the
  **root `xmake.lua`** (simplest) vs a separate `docs/tool/xmake.lua`
  built with `-P`. Default: root `xmake.lua` with `set_default(false)`,
  because it keeps a single build entry point and `set_default(false)`
  already isolates it from engine builds.

### Behavior

1. Recursively walk `docs/content/**/*.md`.
2. For each file, split off a leading front-matter block delimited by
   `---` lines and parse it with a **minimal own parser**: simple
   `key: value` lines, keys `title`, `section`, `nav_order`. No YAML lib.
3. Render the Markdown body to HTML with **md4c** (`md_html`), with GFM
   tables enabled (`MD_FLAG_TABLES` and the other GFM flags as needed).
4. Build the navigation model: group pages by `section`, order within a
   section by `nav_order`; `index.md` is the home page and is excluded
   from the sidebar. Section order is a small fixed priority list in the
   tool (e.g. `Start`, `Reference`, `Examples`), matching the owl/nottario
   model; unknown sections fall to the end.
5. Load `templates/layout.html` and substitute placeholders by plain
   string replacement: `{{title}}`, `{{nav}}`, `{{content}}`,
   `{{base_url}}`. No template engine.
6. Write each page to `docs/dist/<relative-path>.html` (e.g.
   `reference/items.md` → `dist/reference/items.html`), and copy
   `docs/static/` into `dist/static/`.

### CLI

```
okinawa-docs --in docs/content --out docs/dist --base-url /okinawa.cpp
```

`--base-url` prefixes every internal link and asset reference so the site
works under the `/okinawa.cpp` Pages subpath. Default `--base-url` is
empty (for local preview at the filesystem root).

## Visual design (impeccable)

`templates/layout.html` and `static/docs.css` are authored from scratch
with the `impeccable` skill. Requirements:

- Okinawa's own identity (uses `assets/project/okinawa_logo.png`), not the
  GitHub-like look of nottario/owl.
- Layout: top bar with brand + GitHub link; left sidebar navigation built
  from the `{{nav}}` placeholder; main article column for `{{content}}`.
- Readable code blocks (the site is code-heavy: tutorials + reference).
- Responsive enough to read on a laptop and a phone.
- Light is the baseline; a dark mode is desirable but optional for the
  first cut (decided during the impeccable pass).

The generator stays agnostic to the design: it only fills the four
placeholders, so the impeccable output can evolve independently.

## Deployment (CI)

New workflow `.github/workflows/docs.yml`:

- **Trigger:** push to `master`.
- **Permissions:** `pages: write`, `id-token: write`.
- **Concurrency:** single-flight `github-pages` environment.
- **Steps:** checkout → `xmake-io/setup-xmake-action` → `xmake build
  okinawa-docs` → run
  `xmake run okinawa-docs --in docs/content --out docs/dist --base-url /okinawa.cpp`
  → `actions/upload-pages-artifact` with `path: docs/dist` →
  `actions/deploy-pages@v5`.

xrepo fetches `md4c` during the build (cache the xmake package dir to keep
CI fast).

## Content, first batch

- `index.md` — what okinawa is, link to getting-started and to the
  tutorials.
- `getting-started.md` — consume the engine as a git submodule, build with
  xmake, a minimal "open a window + render an item" snippet.
- `reference/*.md` (8 pages) — curated per subsystem: the public classes,
  what they're for, the handful of methods that matter, a short snippet.
- `examples/objects-on-the-fly.md` — create `OkItem` cubes at runtime, set
  wireframe/position, `attachTo` one to another, rotate per frame in the
  step callback. Sourced almost verbatim from wadviewer's `main.cpp`
  (the two demo cubes).
- `examples/texture-viewer.md` — build an on-screen texture-preview
  overlay, cycle textures with a key. Sourced from wadviewer's
  `GUI`/`texturePreviewElement`.

## Out-of-band actions (require the user — not done by the agent)

1. Enable **GitHub Pages → Source: GitHub Actions** in the repo settings.
2. Confirm the repo is `okinawa-dev/okinawa.cpp` so `--base-url
   /okinawa.cpp` and the Pages URL are correct. (Verified from
   `git remote`: `git@github.com:okinawa-dev/okinawa.cpp.git`.)

## Tracking

Tracked in Nottario under role **okinawa** (work in the okinawa.cpp repo).

## Build / work order

1. This spec.
2. Visual design with `impeccable` (`layout.html` + `docs.css`).
3. The C++ generator (`docs/tool`, xmake target, md4c).
4. Content first batch (index, getting-started, reference, 2 tutorials).
5. CI workflow `docs.yml`.
