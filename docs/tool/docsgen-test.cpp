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
