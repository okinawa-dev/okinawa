#include "docsgen.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("docsgen scaffold links", "[docs]") {
  REQUIRE(docsgen::outputPathFor("x") == "x");
}
