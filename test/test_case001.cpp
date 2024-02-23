#define CATCH_CONFIG_MAIN

#include "../include/catch2.hpp"
#include "../include/string.hpp"

TEST_CASE("testing constructors and comparing", "[single-file]") {
    my::String str = "hello, world";
    my::String str_copy = str;
    REQUIRE(1 == 1);
    REQUIRE(str == "hello, world");
    REQUIRE(str_copy == "hello, world");
    REQUIRE(str_copy == str);

    my::String cum = "123";
    cum.pop_back();
    REQUIRE(cum == "12");

    str.clear();
    str.push_back('1');
    REQUIRE(str == "1");
}

TEST_CASE("testing memory", "[single-file]") {
    my::String str = "hello";
    REQUIRE(str.length() == 5);
    str.pop_back();
    REQUIRE(str == "hell");
    REQUIRE(str.length() == 4);
    str.pop_back();
    str.pop_back();
    str.pop_back();
    str.pop_back();
    my::String str_copy = str;
    str.clear();
    REQUIRE(str == str_copy);
}
