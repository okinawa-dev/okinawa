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
