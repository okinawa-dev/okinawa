# Okinawa Documentation Site Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Markdown-driven documentation website for the Okinawa engine, rendered by a small C++/xmake static-site generator and deployed to GitHub Pages at `https://okinawa-dev.github.io/okinawa.cpp/`.

**Architecture:** A standalone C++17 generator (`docs/tool`) parses Markdown files with front-matter, renders them to HTML with md4c, fills a single HTML template, and writes a static site to `docs/dist/`. The generator does NOT link the engine. Pure helpers live in `docsgen.{hpp,cpp}` and are unit-tested with Catch2; `main.cpp` wires CLI + filesystem + md4c. The visual template/CSS are authored separately with the `impeccable` skill. A GitHub Actions workflow builds and deploys on push to `master`.

**Tech Stack:** C++17, xmake (build + xrepo deps), md4c (Markdown→HTML), Catch2 v3 (tests), GitHub Actions + Pages.

**Spec:** `docs/superpowers/specs/2026-06-18-okinawa-docs-site-design.md`

**Repo:** all work in `okinawa-dev/okinawa.cpp` (the standalone checkout at `/Users/ialonso/Projects/neverbot/okinawa.cpp`). The engine code style avoids C++17 library features; the docs **tool** is build-time tooling, separate from the engine, and may use `<filesystem>` and standard C++17 freely.

---

## File Structure

| File | Responsibility |
| --- | --- |
| `docs/tool/docsgen.hpp` | Data model (`FrontMatter`, `Page`) + pure helper declarations |
| `docs/tool/docsgen.cpp` | Pure helpers: front-matter parse, path mapping, nav build, base-url rewrite, template fill, md4c render |
| `docs/tool/docsgen-test.cpp` | Catch2 unit tests for the pure helpers |
| `docs/tool/main.cpp` | CLI parsing, filesystem walk, render pipeline, output writing, static copy |
| `docs/templates/layout.html` | Single page template (placeholder in Task 1; final from impeccable in Task 7) |
| `docs/static/docs.css` | Site styles (placeholder in Task 1; final from impeccable in Task 7) |
| `docs/content/**/*.md` | Markdown sources (Task 8) |
| `.github/workflows/docs.yml` | Build + deploy to Pages (Task 9) |
| `xmake.lua` | Add `okinawa-docs` + `okinawa-docs-tests` targets, `md4c` require |
| `.gitignore` | Ignore `docs/dist/` |

**Render pipeline (locked here, referenced by tasks):** for each page —
1. `nav = buildNav(allPages, page.outRelPath)` → root-relative links.
2. `navF = applyBaseUrl(nav, baseUrl)`.
3. `contentHtml = renderMarkdown(page.body)`; `contentF = applyBaseUrl(contentHtml, baseUrl)`.
4. `full = renderTemplate(template, page.title, navF, contentF, baseUrl)` — the template's own asset refs use the literal `{{base_url}}` placeholder, so they are resolved here and never double-prefixed.
5. Write `full` to `out/<page.outRelPath>`.

---

## Task 1: Scaffold docs tree, xmake targets, gitignore

**Files:**
- Create: `docs/tool/docsgen.hpp`
- Create: `docs/tool/docsgen.cpp` (stub bodies)
- Create: `docs/tool/docsgen-test.cpp` (one trivial passing test)
- Create: `docs/tool/main.cpp` (prints usage)
- Create: `docs/templates/layout.html` (minimal placeholder)
- Create: `docs/static/docs.css` (empty placeholder)
- Modify: `xmake.lua`
- Modify: `.gitignore`

- [ ] **Step 1: Create the header with the full data model and declarations**

`docs/tool/docsgen.hpp`:

```cpp
#ifndef OKINAWA_DOCS_DOCSGEN_HPP
#define OKINAWA_DOCS_DOCSGEN_HPP

#include <string>
#include <vector>

namespace docsgen {

struct FrontMatter {
  std::string title;
  std::string section;
  int         navOrder;
  bool        hasNavOrder;
};

struct Page {
  std::string srcRelPath;  // e.g. "reference/items.md"
  std::string outRelPath;  // e.g. "reference/items.html"
  std::string title;
  std::string section;
  int         navOrder;
  bool        hasNavOrder;
  std::string body;        // markdown body (front-matter stripped)
};

// Split a leading "---\n ... \n---\n" front-matter block. Returns true only
// when a complete block (opening and closing "---" lines) is present, in which
// case fmOut holds the parsed keys (title, section, nav_order) and bodyOut is
// the text after the closing delimiter. On false, fmOut is unspecified and
// bodyOut == content. Tolerates a UTF-8 BOM and CRLF line endings.
bool parseFrontMatter(const std::string &content, FrontMatter &fmOut,
                      std::string &bodyOut);

// Map a source markdown path to its output html path:
//   "index.md" -> "index.html", "reference/items.md" -> "reference/items.html".
std::string outputPathFor(const std::string &srcRelPath);

// Sidebar ordering rank for a section. Lower sorts first.
// Start(0), Reference(1), Examples(2); anything else -> 1000.
int sectionRank(const std::string &section);

// Build sidebar nav HTML from all pages, grouped by section (ranked, then
// alphabetical), ordered by navOrder within a section. The home page
// (outRelPath == "index.html") is excluded. The page whose outRelPath ==
// currentOutPath gets class="active". Links are root-relative ("/foo.html");
// applyBaseUrl prefixes them afterwards.
std::string buildNav(const std::vector<Page> &pages,
                     const std::string &currentOutPath);

// Prefix every root-relative href="/..." and src="/..." with baseUrl.
// Leaves href="#...", protocol-relative "//...", and external "http..." alone.
// baseUrl "" returns html unchanged.
std::string applyBaseUrl(const std::string &html, const std::string &baseUrl);

// Replace all occurrences of {{title}}, {{nav}}, {{content}}, {{base_url}}.
std::string renderTemplate(const std::string &tmpl, const std::string &title,
                           const std::string &nav, const std::string &content,
                           const std::string &baseUrl);

// Render a Markdown fragment to HTML via md4c (GitHub dialect: tables,
// strikethrough, permissive autolinks, task lists).
std::string renderMarkdown(const std::string &markdown);

}  // namespace docsgen

#endif  // OKINAWA_DOCS_DOCSGEN_HPP
```

- [ ] **Step 2: Create stub implementations that compile**

`docs/tool/docsgen.cpp`:

```cpp
#include "docsgen.hpp"

namespace docsgen {

bool parseFrontMatter(const std::string &content, FrontMatter &fmOut,
                      std::string &bodyOut) {
  (void)fmOut;
  bodyOut = content;
  return false;
}

std::string outputPathFor(const std::string &srcRelPath) { return srcRelPath; }

int sectionRank(const std::string &section) {
  (void)section;
  return 1000;
}

std::string buildNav(const std::vector<Page> &pages,
                     const std::string &currentOutPath) {
  (void)pages;
  (void)currentOutPath;
  return "";
}

std::string applyBaseUrl(const std::string &html, const std::string &baseUrl) {
  (void)baseUrl;
  return html;
}

std::string renderTemplate(const std::string &tmpl, const std::string &title,
                           const std::string &nav, const std::string &content,
                           const std::string &baseUrl) {
  (void)title;
  (void)nav;
  (void)content;
  (void)baseUrl;
  return tmpl;
}

std::string renderMarkdown(const std::string &markdown) { return markdown; }

}  // namespace docsgen
```

- [ ] **Step 3: Create a trivial passing test**

`docs/tool/docsgen-test.cpp`:

```cpp
#include "docsgen.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("docsgen scaffold links", "[docs]") {
  REQUIRE(docsgen::outputPathFor("x") == "x");
}
```

- [ ] **Step 4: Create the main entry stub**

`docs/tool/main.cpp`:

```cpp
#include <cstdio>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  std::printf(
      "okinawa-docs: usage: okinawa-docs --in docs/content --out docs/dist "
      "[--base-url /okinawa.cpp] [--template docs/templates/layout.html] "
      "[--static docs/static]\n");
  return 0;
}
```

- [ ] **Step 5: Create a minimal placeholder template and empty stylesheet**

`docs/templates/layout.html` (replaced by the impeccable design in Task 7; kept minimal so the generator runs end-to-end before then):

```html
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{{title}} · Okinawa</title>
  <link rel="stylesheet" href="{{base_url}}/static/docs.css">
</head>
<body>
  <header><a href="{{base_url}}/index.html">Okinawa</a></header>
  <nav>{{nav}}</nav>
  <main>{{content}}</main>
</body>
</html>
```

`docs/static/docs.css`:

```css
/* Placeholder. Replaced by the impeccable design in Task 7. */
```

- [ ] **Step 6: Add the xmake targets**

In `xmake.lua`, add `add_requires("md4c")` next to the other root requires (after line `add_requires("catch2")`):

```lua
add_requires("md4c")                -- Markdown->HTML for the docs site generator
```

Then append the two targets after the `okinawa_test` target (before the `coverage` task):

```lua
-- =========================================================================
-- Documentation site generator (docs/tool). Standalone: does NOT link the
-- engine. Built explicitly with `xmake build okinawa-docs`; a bare `xmake`
-- skips it (set_default false). Run from the project root so docs/ resolves.
-- =========================================================================
target("okinawa-docs")
    set_kind("binary")
    set_default(false)
    add_files("docs/tool/main.cpp", "docs/tool/docsgen.cpp")
    add_packages("md4c")
    set_rundir("$(projectdir)")

target("okinawa-docs-tests")
    set_kind("binary")
    set_default(false)
    add_files("docs/tool/docsgen-test.cpp", "docs/tool/docsgen.cpp")
    add_packages("catch2", "md4c")
    set_rundir("$(projectdir)")
    add_tests("docs")
```

- [ ] **Step 7: Ignore the generated output**

Append to `.gitignore`:

```
# docs site (generated)
docs/dist/
```

- [ ] **Step 8: Verify build isolation — a bare `xmake` skips the docs targets**

Run: `xmake build okinawa 2>&1 | tail -5`
Expected: the engine builds; no `okinawa-docs` compilation occurs.

- [ ] **Step 9: Verify the docs targets compile and the test passes**

Run: `xmake build okinawa-docs okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: both targets link; the test binary reports `All tests passed (1 assertion in 1 test case)`.

- [ ] **Step 10: Verify the generator stub runs**

Run: `xmake run okinawa-docs`
Expected: prints the usage line.

- [ ] **Step 11: Commit**

```bash
git add docs/tool docs/templates docs/static xmake.lua .gitignore
git commit -m "build(docs): scaffold the C++ docs-site generator and xmake targets"
```

---

## Task 2: Front-matter parser

**Files:**
- Modify: `docs/tool/docsgen.cpp`
- Modify: `docs/tool/docsgen-test.cpp`

- [ ] **Step 1: Write the failing tests**

Replace the body of `docs/tool/docsgen-test.cpp` with:

```cpp
#include "docsgen.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("parseFrontMatter reads keys and strips the block", "[docs]") {
  std::string content =
      "---\n"
      "title: Items\n"
      "section: Reference\n"
      "nav_order: 2\n"
      "---\n"
      "# Items\n\nBody text.\n";
  docsgen::FrontMatter fm;
  std::string body;
  bool ok = docsgen::parseFrontMatter(content, fm, body);
  REQUIRE(ok);
  REQUIRE(fm.title == "Items");
  REQUIRE(fm.section == "Reference");
  REQUIRE(fm.navOrder == 2);
  REQUIRE(fm.hasNavOrder);
  REQUIRE(body == "# Items\n\nBody text.\n");
}

TEST_CASE("parseFrontMatter returns false without a block", "[docs]") {
  std::string content = "# No front matter\n";
  docsgen::FrontMatter fm;
  std::string body;
  bool ok = docsgen::parseFrontMatter(content, fm, body);
  REQUIRE_FALSE(ok);
  REQUIRE(body == content);
}

TEST_CASE("parseFrontMatter tolerates CRLF and missing nav_order", "[docs]") {
  std::string content = "---\r\ntitle: Home\r\n---\r\nHello\r\n";
  docsgen::FrontMatter fm;
  std::string body;
  bool ok = docsgen::parseFrontMatter(content, fm, body);
  REQUIRE(ok);
  REQUIRE(fm.title == "Home");
  REQUIRE_FALSE(fm.hasNavOrder);
  REQUIRE(body == "Hello\r\n");
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: FAIL (stub returns false / empty values).

- [ ] **Step 3: Implement the parser plus shared string helpers**

In `docs/tool/docsgen.cpp`, add these file-local helpers at the top (after the include) — they are reused by later tasks:

```cpp
#include <cstdlib>

namespace {

std::string trim(const std::string &s) {
  std::size_t a = 0;
  std::size_t b = s.size();
  while (a < b && (s[a] == ' ' || s[a] == '\t')) a++;
  while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r'))
    b--;
  return s.substr(a, b - a);
}

}  // namespace
```

Then replace the `parseFrontMatter` stub body with:

```cpp
bool parseFrontMatter(const std::string &content, FrontMatter &fmOut,
                      std::string &bodyOut) {
  fmOut = FrontMatter();
  fmOut.navOrder = 0;
  fmOut.hasNavOrder = false;

  std::size_t pos = 0;
  if (content.compare(0, 3, "\xEF\xBB\xBF") == 0) pos = 3;  // skip BOM

  std::size_t firstNl = content.find('\n', pos);
  if (firstNl == std::string::npos) {
    bodyOut = content;
    return false;
  }
  std::string firstLine = trim(content.substr(pos, firstNl - pos));
  if (firstLine != "---") {
    bodyOut = content;
    return false;
  }

  std::size_t lineStart = firstNl + 1;
  while (lineStart <= content.size()) {
    std::size_t nl = content.find('\n', lineStart);
    std::string line = (nl == std::string::npos)
                           ? content.substr(lineStart)
                           : content.substr(lineStart, nl - lineStart);
    if (trim(line) == "---") {
      bodyOut = (nl == std::string::npos) ? "" : content.substr(nl + 1);
      return true;
    }
    std::size_t colon = line.find(':');
    if (colon != std::string::npos) {
      std::string key = trim(line.substr(0, colon));
      std::string val = trim(line.substr(colon + 1));
      if (key == "title") {
        fmOut.title = val;
      } else if (key == "section") {
        fmOut.section = val;
      } else if (key == "nav_order") {
        fmOut.navOrder = std::atoi(val.c_str());
        fmOut.hasNavOrder = true;
      }
    }
    if (nl == std::string::npos) break;
    lineStart = nl + 1;
  }

  bodyOut = content;  // no closing delimiter -> not a valid block
  return false;
}
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: PASS (all assertions).

- [ ] **Step 5: Commit**

```bash
git add docs/tool/docsgen.cpp docs/tool/docsgen-test.cpp
git commit -m "feat(docs): parse markdown front-matter (title/section/nav_order)"
```

---

## Task 3: Output path mapping and section ranking

**Files:**
- Modify: `docs/tool/docsgen.cpp`
- Modify: `docs/tool/docsgen-test.cpp`

- [ ] **Step 1: Write the failing tests**

Append to `docs/tool/docsgen-test.cpp`:

```cpp
TEST_CASE("outputPathFor swaps .md for .html", "[docs]") {
  REQUIRE(docsgen::outputPathFor("index.md") == "index.html");
  REQUIRE(docsgen::outputPathFor("reference/items.md") ==
          "reference/items.html");
  REQUIRE(docsgen::outputPathFor("examples/texture-viewer.md") ==
          "examples/texture-viewer.html");
}

TEST_CASE("sectionRank orders known sections first", "[docs]") {
  REQUIRE(docsgen::sectionRank("Start") < docsgen::sectionRank("Reference"));
  REQUIRE(docsgen::sectionRank("Reference") < docsgen::sectionRank("Examples"));
  REQUIRE(docsgen::sectionRank("Examples") < docsgen::sectionRank("Misc"));
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: FAIL (stub `outputPathFor` returns input; `sectionRank` always 1000).

- [ ] **Step 3: Implement both functions**

Replace the `outputPathFor` and `sectionRank` stub bodies in `docs/tool/docsgen.cpp`:

```cpp
std::string outputPathFor(const std::string &srcRelPath) {
  const std::string ext = ".md";
  if (srcRelPath.size() >= ext.size() &&
      srcRelPath.compare(srcRelPath.size() - ext.size(), ext.size(), ext) ==
          0) {
    return srcRelPath.substr(0, srcRelPath.size() - ext.size()) + ".html";
  }
  return srcRelPath;
}

int sectionRank(const std::string &section) {
  if (section == "Start") return 0;
  if (section == "Reference") return 1;
  if (section == "Examples") return 2;
  return 1000;
}
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add docs/tool/docsgen.cpp docs/tool/docsgen-test.cpp
git commit -m "feat(docs): map .md->.html output paths and rank nav sections"
```

---

## Task 4: Sidebar navigation builder

**Files:**
- Modify: `docs/tool/docsgen.cpp`
- Modify: `docs/tool/docsgen-test.cpp`

- [ ] **Step 1: Write the failing test**

Append to `docs/tool/docsgen-test.cpp`:

```cpp
static docsgen::Page mkPage(const std::string &out, const std::string &title,
                            const std::string &section, int navOrder) {
  docsgen::Page p;
  p.outRelPath = out;
  p.title = title;
  p.section = section;
  p.navOrder = navOrder;
  p.hasNavOrder = true;
  return p;
}

TEST_CASE("buildNav groups, orders, excludes home, marks active", "[docs]") {
  std::vector<docsgen::Page> pages;
  pages.push_back(mkPage("index.html", "Home", "", 0));
  pages.push_back(mkPage("reference/items.html", "Items", "Reference", 2));
  pages.push_back(mkPage("getting-started.html", "Getting started", "Start", 1));
  pages.push_back(mkPage("reference/core.html", "Core", "Reference", 1));

  std::string nav = docsgen::buildNav(pages, "reference/items.html");

  // Home is excluded.
  REQUIRE(nav.find("Home") == std::string::npos);
  // Start section appears before Reference section.
  REQUIRE(nav.find("Start") < nav.find("Reference"));
  // Within Reference, Core (nav_order 1) precedes Items (nav_order 2).
  REQUIRE(nav.find(">Core<") < nav.find(">Items<"));
  // The current page is marked active.
  REQUIRE(nav.find("href=\"/reference/items.html\" class=\"active\"") !=
          std::string::npos);
  // Links are root-relative.
  REQUIRE(nav.find("href=\"/getting-started.html\"") != std::string::npos);
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: FAIL (stub returns empty string).

- [ ] **Step 3: Implement `buildNav` plus an html-escape helper and comparator**

In `docs/tool/docsgen.cpp`, add to the anonymous namespace (next to `trim`):

```cpp
#include <algorithm>

std::string htmlEscape(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (std::size_t i = 0; i < s.size(); i++) {
    char c = s[i];
    if (c == '&') {
      out += "&amp;";
    } else if (c == '<') {
      out += "&lt;";
    } else if (c == '>') {
      out += "&gt;";
    } else if (c == '"') {
      out += "&quot;";
    } else {
      out += c;
    }
  }
  return out;
}

bool pageLess(const docsgen::Page &a, const docsgen::Page &b) {
  int ra = docsgen::sectionRank(a.section);
  int rb = docsgen::sectionRank(b.section);
  if (ra != rb) return ra < rb;
  if (a.section != b.section) return a.section < b.section;
  if (a.navOrder != b.navOrder) return a.navOrder < b.navOrder;
  return a.title < b.title;
}
```

Replace the `buildNav` stub body:

```cpp
std::string buildNav(const std::vector<Page> &pages,
                     const std::string &currentOutPath) {
  std::vector<Page> items;
  for (std::size_t i = 0; i < pages.size(); i++) {
    if (pages[i].outRelPath == "index.html") continue;
    items.push_back(pages[i]);
  }
  std::stable_sort(items.begin(), items.end(), pageLess);

  std::string html;
  std::string currentSection;
  bool open = false;
  for (std::size_t i = 0; i < items.size(); i++) {
    const Page &p = items[i];
    if (i == 0 || p.section != currentSection) {
      if (open) html += "</ul>\n";
      currentSection = p.section;
      html += "<div class=\"nav-section\">" + htmlEscape(p.section) +
              "</div>\n<ul>\n";
      open = true;
    }
    std::string cls =
        (p.outRelPath == currentOutPath) ? " class=\"active\"" : "";
    html += "<li><a href=\"/" + p.outRelPath + "\"" + cls + ">" +
            htmlEscape(p.title) + "</a></li>\n";
  }
  if (open) html += "</ul>\n";
  return html;
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add docs/tool/docsgen.cpp docs/tool/docsgen-test.cpp
git commit -m "feat(docs): build the grouped, ordered sidebar navigation"
```

---

## Task 5: Base-URL rewriting and template filling

**Files:**
- Modify: `docs/tool/docsgen.cpp`
- Modify: `docs/tool/docsgen-test.cpp`

- [ ] **Step 1: Write the failing tests**

Append to `docs/tool/docsgen-test.cpp`:

```cpp
TEST_CASE("applyBaseUrl prefixes only root-relative urls", "[docs]") {
  std::string in =
      "<a href=\"/getting-started.html\">x</a>"
      "<img src=\"/static/logo.png\">"
      "<a href=\"#anchor\">y</a>"
      "<a href=\"https://x.test/p\">z</a>"
      "<a href=\"//cdn.test/a\">w</a>";
  std::string out = docsgen::applyBaseUrl(in, "/okinawa.cpp");
  REQUIRE(out.find("href=\"/okinawa.cpp/getting-started.html\"") !=
          std::string::npos);
  REQUIRE(out.find("src=\"/okinawa.cpp/static/logo.png\"") !=
          std::string::npos);
  REQUIRE(out.find("href=\"#anchor\"") != std::string::npos);
  REQUIRE(out.find("href=\"https://x.test/p\"") != std::string::npos);
  REQUIRE(out.find("href=\"//cdn.test/a\"") != std::string::npos);
}

TEST_CASE("applyBaseUrl with empty base is a no-op", "[docs]") {
  std::string in = "<a href=\"/x.html\">x</a>";
  REQUIRE(docsgen::applyBaseUrl(in, "") == in);
}

TEST_CASE("renderTemplate fills all placeholders", "[docs]") {
  std::string tmpl =
      "<title>{{title}}</title><nav>{{nav}}</nav><main>{{content}}</main>"
      "<link href=\"{{base_url}}/static/docs.css\">";
  std::string out = docsgen::renderTemplate(tmpl, "Items", "<ul></ul>", "<p>x</p>",
                                            "/okinawa.cpp");
  REQUIRE(out ==
          "<title>Items</title><nav><ul></ul></nav><main><p>x</p></main>"
          "<link href=\"/okinawa.cpp/static/docs.css\">");
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: FAIL (stubs are no-ops / return template unchanged).

- [ ] **Step 3: Implement the helpers**

In `docs/tool/docsgen.cpp`, add to the anonymous namespace:

```cpp
std::string replaceAll(std::string s, const std::string &from,
                       const std::string &to) {
  if (from.empty()) return s;
  std::size_t pos = 0;
  while ((pos = s.find(from, pos)) != std::string::npos) {
    s.replace(pos, from.size(), to);
    pos += to.size();
  }
  return s;
}

// Insert baseUrl before the leading '/' of every `token + "/..."` whose value
// is root-relative (single leading slash, not "//").
std::string prefixRootRel(const std::string &html, const std::string &token,
                          const std::string &baseUrl) {
  std::string out;
  out.reserve(html.size() + 64);
  std::size_t i = 0;
  while (i < html.size()) {
    std::size_t hit = html.find(token, i);
    if (hit == std::string::npos) {
      out += html.substr(i);
      break;
    }
    out += html.substr(i, hit - i);
    out += token;
    std::size_t after = hit + token.size();
    if (after < html.size() && html[after] == '/' &&
        !(after + 1 < html.size() && html[after + 1] == '/')) {
      out += baseUrl;
    }
    i = after;
  }
  return out;
}
```

Replace the `applyBaseUrl` and `renderTemplate` stub bodies:

```cpp
std::string applyBaseUrl(const std::string &html, const std::string &baseUrl) {
  if (baseUrl.empty()) return html;
  std::string out = prefixRootRel(html, "href=\"", baseUrl);
  out = prefixRootRel(out, "src=\"", baseUrl);
  return out;
}

std::string renderTemplate(const std::string &tmpl, const std::string &title,
                           const std::string &nav, const std::string &content,
                           const std::string &baseUrl) {
  std::string out = tmpl;
  out = replaceAll(out, "{{title}}", title);
  out = replaceAll(out, "{{nav}}", nav);
  out = replaceAll(out, "{{content}}", content);
  out = replaceAll(out, "{{base_url}}", baseUrl);
  return out;
}
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add docs/tool/docsgen.cpp docs/tool/docsgen-test.cpp
git commit -m "feat(docs): base-url link rewriting and template placeholder fill"
```

---

## Task 6: Markdown rendering and the main pipeline

**Files:**
- Modify: `docs/tool/docsgen.cpp`
- Modify: `docs/tool/docsgen-test.cpp`
- Modify: `docs/tool/main.cpp`

- [ ] **Step 1: Write the failing markdown test**

Append to `docs/tool/docsgen-test.cpp`:

```cpp
TEST_CASE("renderMarkdown emits html, including GFM tables", "[docs]") {
  std::string h1 = docsgen::renderMarkdown("# Hello\n");
  REQUIRE(h1.find("<h1") != std::string::npos);
  REQUIRE(h1.find("Hello") != std::string::npos);

  std::string table = docsgen::renderMarkdown(
      "| A | B |\n| - | - |\n| 1 | 2 |\n");
  REQUIRE(table.find("<table") != std::string::npos);
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: FAIL (stub returns the markdown unchanged; no `<h1`/`<table`).

- [ ] **Step 3: Implement `renderMarkdown` with md4c**

In `docs/tool/docsgen.cpp`, add the include near the top (with the other includes):

```cpp
#include <md4c-html.h>
```

Add to the anonymous namespace:

```cpp
void mdSink(const MD_CHAR *text, MD_SIZE size, void *userdata) {
  std::string *out = static_cast<std::string *>(userdata);
  out->append(text, size);
}
```

Replace the `renderMarkdown` stub body:

```cpp
std::string renderMarkdown(const std::string &markdown) {
  std::string out;
  unsigned parserFlags = MD_DIALECT_GITHUB;
  md_html(markdown.data(), static_cast<MD_SIZE>(markdown.size()), mdSink, &out,
          parserFlags, 0);
  return out;
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `xmake build okinawa-docs-tests && xmake run okinawa-docs-tests`
Expected: PASS.

- [ ] **Step 5: Implement the full generator in `main.cpp`**

Replace `docs/tool/main.cpp` entirely:

```cpp
#include "docsgen.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

std::string readFile(const fs::path &p) {
  std::ifstream in(p, std::ios::binary);
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

void writeFile(const fs::path &p, const std::string &data) {
  fs::create_directories(p.parent_path());
  std::ofstream out(p, std::ios::binary);
  out << data;
}

// Default title from a path: "reference/items.md" -> "items".
std::string titleFromPath(const std::string &srcRelPath) {
  std::string stem = fs::path(srcRelPath).stem().string();
  return stem;
}

std::string argValue(int argc, char **argv, const std::string &flag,
                     const std::string &fallback) {
  for (int i = 1; i + 1 < argc; i++) {
    if (flag == argv[i]) return argv[i + 1];
  }
  return fallback;
}

}  // namespace

int main(int argc, char **argv) {
  std::string inDir = argValue(argc, argv, "--in", "docs/content");
  std::string outDir = argValue(argc, argv, "--out", "docs/dist");
  std::string baseUrl = argValue(argc, argv, "--base-url", "");
  std::string templatePath =
      argValue(argc, argv, "--template", "docs/templates/layout.html");
  std::string staticDir = argValue(argc, argv, "--static", "docs/static");

  if (!fs::exists(inDir)) {
    std::fprintf(stderr, "okinawa-docs: input dir not found: %s\n",
                 inDir.c_str());
    return 1;
  }
  if (!fs::exists(templatePath)) {
    std::fprintf(stderr, "okinawa-docs: template not found: %s\n",
                 templatePath.c_str());
    return 1;
  }

  std::string tmpl = readFile(templatePath);

  // First pass: collect every page.
  std::vector<docsgen::Page> pages;
  fs::path inRoot(inDir);
  for (fs::recursive_directory_iterator it(inRoot), end; it != end; ++it) {
    if (!it->is_regular_file()) continue;
    if (it->path().extension() != ".md") continue;

    std::string rel = fs::relative(it->path(), inRoot).generic_string();
    std::string content = readFile(it->path());

    docsgen::FrontMatter fm;
    std::string body;
    bool hasFm = docsgen::parseFrontMatter(content, fm, body);

    docsgen::Page page;
    page.srcRelPath = rel;
    page.outRelPath = docsgen::outputPathFor(rel);
    page.title = (hasFm && !fm.title.empty()) ? fm.title : titleFromPath(rel);
    page.section = hasFm ? fm.section : "";
    page.navOrder = (hasFm && fm.hasNavOrder) ? fm.navOrder : 1000;
    page.hasNavOrder = hasFm && fm.hasNavOrder;
    page.body = hasFm ? body : content;
    pages.push_back(page);
  }

  // Second pass: render each page (see the pipeline in the plan header).
  fs::path outRoot(outDir);
  fs::remove_all(outRoot);
  for (std::size_t i = 0; i < pages.size(); i++) {
    const docsgen::Page &page = pages[i];
    std::string nav =
        docsgen::applyBaseUrl(docsgen::buildNav(pages, page.outRelPath),
                              baseUrl);
    std::string content =
        docsgen::applyBaseUrl(docsgen::renderMarkdown(page.body), baseUrl);
    std::string full =
        docsgen::renderTemplate(tmpl, page.title, nav, content, baseUrl);
    writeFile(outRoot / page.outRelPath, full);
  }

  // Copy static assets to out/static.
  if (fs::exists(staticDir)) {
    fs::copy(staticDir, outRoot / "static",
             fs::copy_options::recursive |
                 fs::copy_options::overwrite_existing);
  }

  std::printf("okinawa-docs: wrote %zu page(s) to %s\n", pages.size(),
              outDir.c_str());
  return 0;
}
```

- [ ] **Step 6: Build and smoke-test on a temporary fixture**

```bash
xmake build okinawa-docs
mkdir -p /tmp/okidocs/content/reference
printf -- '---\ntitle: Home\nsection: Start\nnav_order: 1\n---\n# Okinawa\n\nWelcome.\n' > /tmp/okidocs/content/index.md
printf -- '---\ntitle: Items\nsection: Reference\nnav_order: 1\n---\n# Items\n\n| A | B |\n| - | - |\n| 1 | 2 |\n' > /tmp/okidocs/content/reference/items.md
xmake run okinawa-docs --in /tmp/okidocs/content --out /tmp/okidocs/dist --base-url /okinawa.cpp --static docs/static
```

Run: `cat /tmp/okidocs/dist/reference/items.html`
Expected: an HTML page whose `<nav>` contains `href="/okinawa.cpp/index.html"` is **absent** (home excluded) but a Reference group with `href="/okinawa.cpp/reference/items.html"` marked `active` is present, and `<main>` contains a `<table>`. The stylesheet link reads `href="/okinawa.cpp/static/docs.css"`.

- [ ] **Step 7: Verify a no-base-url local build keeps links root-relative**

Run: `xmake run okinawa-docs --in /tmp/okidocs/content --out /tmp/okidocs/dist-local --static docs/static && grep -o 'href="/[a-z]*' /tmp/okidocs/dist-local/index.html | head -1`
Expected: links start with `/` and are NOT prefixed (e.g. `href="/reference`).

- [ ] **Step 8: Commit**

```bash
git add docs/tool/docsgen.cpp docs/tool/docsgen-test.cpp docs/tool/main.cpp
git commit -m "feat(docs): render markdown with md4c and wire the generator pipeline"
```

---

## Task 7: Visual design with the impeccable skill

**This task is interactive/visual — do it in the main session with the `impeccable` skill, not a headless subagent.**

**Files:**
- Modify: `docs/templates/layout.html`
- Modify: `docs/static/docs.css`
- Create: `docs/static/okinawa_logo.png` (copied from `assets/project/okinawa_logo.png`)
- Create: `docs/static/favicon.svg` (or `.png`) as the design dictates

- [ ] **Step 1: Copy the logo into the site assets**

```bash
cp assets/project/okinawa_logo.png docs/static/okinawa_logo.png
```

- [ ] **Step 2: Invoke the impeccable skill to author the template and CSS**

Requirements to hand to the skill:
- Single template `docs/templates/layout.html` that keeps the four placeholders the generator fills verbatim: `{{title}}`, `{{nav}}`, `{{content}}`, `{{base_url}}`.
- Okinawa's own visual identity (use `{{base_url}}/static/okinawa_logo.png` for the brand), NOT the GitHub-like look of nottario/owl.
- Layout: top bar (brand + a GitHub link to `https://github.com/okinawa-dev/okinawa.cpp`), left sidebar rendering `{{nav}}` (the generator emits `<div class="nav-section">` + `<ul><li><a>` with `class="active"` on the current page — style these), main article column for `{{content}}`.
- Readable, scrollable code blocks (the site is code-heavy).
- Responsive: usable on laptop and phone (sidebar collapses or stacks on narrow screens).
- Light theme baseline; dark mode optional.
- All asset references in the template use the `{{base_url}}` prefix (e.g. `href="{{base_url}}/static/docs.css"`) so they resolve under the Pages subpath.

- [ ] **Step 3: Rebuild the smoke fixture and review in a browser**

```bash
xmake run okinawa-docs --in /tmp/okidocs/content --out /tmp/okidocs/dist --static docs/static
open /tmp/okidocs/dist/index.html
```

Expected: the page renders with the new design; sidebar nav, active state, and code blocks look correct.

- [ ] **Step 4: Commit**

```bash
git add docs/templates/layout.html docs/static
git commit -m "feat(docs): design the documentation site shell (impeccable)"
```

---

## Task 8: Author the content (first batch)

**Files (all Create):**
- `docs/content/index.md`
- `docs/content/getting-started.md`
- `docs/content/reference/core.md`
- `docs/content/reference/items.md`
- `docs/content/reference/scene.md`
- `docs/content/reference/math.md`
- `docs/content/reference/input.md`
- `docs/content/reference/handlers.md`
- `docs/content/reference/config.md`
- `docs/content/reference/mcp.md`
- `docs/content/examples/objects-on-the-fly.md`
- `docs/content/examples/texture-viewer.md`

Each file begins with front-matter. Sections: `Start` (index, getting-started), `Reference` (the 8 reference pages), `Examples` (the 2 tutorials). `index.md` uses `section: Start, nav_order: 0` (it is the home page and is excluded from the sidebar, but the front-matter keeps the title/section consistent).

- [ ] **Step 1: Write the home and getting-started pages**

`docs/content/index.md`:

```markdown
---
title: Okinawa
section: Start
nav_order: 0
---

# Okinawa

Okinawa is an in-house C++/OpenGL 3D engine. It provides a scene graph of
items and item groups, cameras, input, math utilities, texture handling, a
configuration store, structured logging, and an optional in-engine MCP server
for agent control.

- New here? Start with [Getting started](/getting-started.html).
- Learn the API in the [Reference](/reference/core.html).
- See it in action in the [Examples](/examples/objects-on-the-fly.html).
```

`docs/content/getting-started.md`:

```markdown
---
title: Getting started
section: Start
nav_order: 1
---

# Getting started

Okinawa builds with [xmake](https://xmake.io); xrepo fetches the third-party
dependencies automatically.

## Consume the engine as a submodule

Vendor the engine at `okinawa.cpp` and build it from source:

```bash
git submodule add https://github.com/okinawa-dev/okinawa.cpp okinawa.cpp
git submodule update --init
```

In your `xmake.lua`:

```lua
includes("okinawa.cpp")

target("myapp")
    set_kind("binary")
    add_files("src/*.cpp")
    add_deps("okinawa")
```

## A minimal window

> NOTE: verify the exact `OkCore` entry points against the headers in
> `src/okinawa/core/` while writing this snippet, and adjust the calls to
> match the real signatures.

```cpp
#include "okinawa/core/core.hpp"

int main() {
  OkCore::initialize();
  // create a scene, add an item, run the loop...
  return 0;
}
```
```

- [ ] **Step 2: Write the 8 reference pages**

> For each reference page, open the corresponding headers under
> `src/okinawa/<subsystem>/` and document the **public** classes and the
> handful of methods a consumer actually calls. Keep each page to: a one-line
> purpose, the key classes, a short table of the important methods, and one
> compilable snippet. Do not paste whole headers. Verify every class/method
> name against the header before writing it.

Create each file with front-matter `section: Reference` and ascending `nav_order` (core 1, items 2, scene 3, math 4, input 5, handlers 6, config 7, mcp 8). Template to follow for every reference page (example shown for `items.md`):

`docs/content/reference/items.md`:

```markdown
---
title: Items
section: Reference
nav_order: 2
---

# Items

`OkItem`, `OkItemGroup` and `OkTexture` (headers in `src/okinawa/item/`) are
the renderable scene-graph building blocks.

## OkItem

A single renderable object: geometry, transform, optional texture, visibility.

| Method | Purpose |
| --- | --- |
| `OkItem(name, vertices, count, ...)` | Construct from a vertex buffer |
| `setPosition(x, y, z)` | Place it in world space |
| `rotate(x, y, z)` | Rotate by Euler angles (radians) |
| `attachTo(parent)` | Parent this item to another (relative transform) |
| `setVisible(bool)` | Show/hide |
| `setWireframe(bool)` | Draw as wireframe |

## OkItemGroup

A named collection of items (e.g. one group per logical object) with
tag-based queries (`getItemsWithTag`, `getItemCountWithTag`).

```cpp
OkItemGroup *group = new OkItemGroup("walls");
group->addItem(item);
```

See the [objects-on-the-fly tutorial](/examples/objects-on-the-fly.html) for a
runnable example.
```

The other seven pages (`core.md`, `scene.md`, `math.md`, `input.md`,
`handlers.md`, `config.md`, `mcp.md`) follow the same shape for their
subsystem's public classes (OkCore/OkCamera/OkObject; OkScene; OkPoint/
OkRotation/OkMath; OkInput/keys; OkSceneHandler/OkTextureHandler; OkConfig;
OkMcpServer).

- [ ] **Step 3: Write the two tutorials**

`docs/content/examples/objects-on-the-fly.md` — create `OkItem` cubes at
runtime, set wireframe/position, `attachTo` one to another, and rotate the
parent each frame in the step callback. Source the code almost verbatim from
the wadviewer demo cubes (`wadviewer/src/main.cpp`, the `item`/`item2`
creation around lines 531-565 and the per-frame `item->rotate(...)` in the
step callback). Front-matter `section: Examples, nav_order: 1`.

`docs/content/examples/texture-viewer.md` — build an on-screen
texture-preview overlay and cycle textures with a key. Source from the
wadviewer `GUI` / `texturePreviewElement` code. Front-matter
`section: Examples, nav_order: 2`.

> Open the referenced wadviewer files and adapt the real code; do not invent
> API. Each tutorial: intro paragraph, the code in stages with prose between,
> and a closing "what you see" line.

- [ ] **Step 4: Build the full site and review**

```bash
xmake run okinawa-docs --in docs/content --out docs/dist --static docs/static
open docs/dist/index.html
```

Expected: 12 pages written; sidebar shows Start / Reference / Examples groups
in that order; every internal link works when navigating locally (links are
root-relative, so use a static server if needed:
`python3 -m http.server -d docs/dist`).

- [ ] **Step 5: Commit**

```bash
git add docs/content
git commit -m "docs(site): author the first content batch (guide, reference, tutorials)"
```

---

## Task 9: GitHub Actions workflow (build + deploy to Pages)

**Files:**
- Create: `.github/workflows/docs.yml`

- [ ] **Step 1: Write the workflow**

`.github/workflows/docs.yml`:

```yaml
name: docs

on:
  push:
    branches: [master]

permissions:
  contents: read
  pages: write
  id-token: write

concurrency:
  group: github-pages
  cancel-in-progress: true

jobs:
  build-deploy:
    runs-on: ubuntu-latest
    environment:
      name: github-pages
      url: ${{ steps.deploy.outputs.page_url }}
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: false

      - name: Set up xmake
        uses: xmake-io/setup-xmake@v1

      - name: Cache xmake packages
        uses: actions/cache@v4
        with:
          path: ~/.xmake
          key: xmake-${{ runner.os }}-${{ hashFiles('xmake.lua') }}

      - name: Build the docs generator
        run: xmake build -y okinawa-docs

      - name: Generate the site
        run: xmake run okinawa-docs --in docs/content --out docs/dist --base-url /okinawa.cpp --static docs/static

      - name: Upload Pages artifact
        uses: actions/upload-pages-artifact@v3
        with:
          path: docs/dist

      - name: Deploy to GitHub Pages
        id: deploy
        uses: actions/deploy-pages@v4
```

> NOTE on versions: `actions/upload-pages-artifact` and `actions/deploy-pages`
> move fast. The spec referenced `@v5`; pin to the latest stable majors at
> implementation time (v3/v4 shown here) and bump if the run reports a
> deprecated action.

- [ ] **Step 2: Lint the YAML locally**

Run: `python3 -c "import yaml,sys; yaml.safe_load(open('.github/workflows/docs.yml')); print('yaml ok')"`
Expected: `yaml ok`

- [ ] **Step 3: Commit**

```bash
git add .github/workflows/docs.yml
git commit -m "ci(docs): build the docs site and deploy to GitHub Pages on push to master"
```

- [ ] **Step 4: Human, out-of-band (NOT the agent)**

1. Repo **Settings → Pages → Build and deployment → Source: GitHub Actions**.
2. Confirm the published URL is `https://okinawa-dev.github.io/okinawa.cpp/`.
   If the repo/org name differs, change `--base-url /okinawa.cpp` in the
   workflow to match the actual Pages subpath.
3. Push `master` (or let the next push trigger the workflow) and confirm the
   `docs` workflow goes green and the site is live.

---

## Done criteria

- `xmake` (bare) builds only the engine; `xmake build okinawa-docs` builds the generator; `xmake run okinawa-docs-tests` is green.
- `xmake run okinawa-docs --in docs/content --out docs/dist --base-url /okinawa.cpp --static docs/static` writes a complete static site to `docs/dist/`.
- The site has a designed shell (impeccable), a getting-started guide, 8 reference pages, and the 2 tutorials, with a working grouped sidebar.
- Pushing to `master` builds and deploys to `https://okinawa-dev.github.io/okinawa.cpp/` (after the human enables Pages → GitHub Actions).

## Nottario mapping

- Task 7 → `3cee94cc` (design, role okinawa)
- Tasks 1-6 → `d73fa6e1` (generator, role okinawa)
- Task 8 → `f70fcd67` (content, role okinawa)
- Task 9 → `80d2df60` (CI, role build)
- Parent feature `8f13894f`. Link each implementation commit to its task and leave a closing comment before `set_state done`.
```
