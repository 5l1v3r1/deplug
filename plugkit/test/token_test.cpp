#include <catch.hpp>
#include "token.h"

using namespace plugkit;

namespace {

TEST_CASE("Token_null") {
  CHECK(Token_null() == 0);
  CHECK(Token_null() == Token_null());
}

TEST_CASE("Token_get(") {
  CHECK(Token_get(nullptr) == Token_null());
  CHECK(Token_get("") == Token_null());
  CHECK(Token_get("a") == Token_get("a"));
  CHECK(Token_get("eth") == Token_get("eth"));
  CHECK(Token_get("[eth]") == Token_get("[eth]"));
  CHECK(Token_get("ipv4") == Token_get("ipv4"));
  CHECK(Token_get("[ipv4]") == Token_get("[ipv4]"));
  CHECK(Token_get("ab") != Token_get("ba"));
}

TEST_CASE("Token_string") {
  CHECK(Token_get(Token_string(Token_get(nullptr))) == Token_get(nullptr));
  CHECK(Token_get(Token_string(Token_get(""))) == Token_get(""));
  CHECK(Token_get(Token_string(Token_get("a"))) == Token_get("a"));
  CHECK(Token_get(Token_string(Token_get("eth"))) == Token_get("eth"));
  CHECK(Token_get(Token_string(Token_get("[eth]"))) == Token_get("[eth]"));
  CHECK(Token_get(Token_string(Token_get("ipv4"))) == Token_get("ipv4"));
  CHECK(Token_get(Token_string(Token_get("[ipv4]"))) == Token_get("[ipv4]"));
}
}
