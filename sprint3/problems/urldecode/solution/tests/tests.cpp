#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    BOOST_TEST(UrlDecode(""sv) == ""s);
    BOOST_TEST(UrlDecode("+"sv) == " "s);
    BOOST_TEST(UrlDecode("-"sv) == "-"s);
    BOOST_TEST(UrlDecode("%20"sv) == " "s);
    BOOST_TEST(UrlDecode("_.~"sv) == "_.~"s);
    BOOST_TEST(UrlDecode("Hello+World%20%21"sv) == "Hello World !"s);
    BOOST_TEST(UrlDecode("%20%20"sv) == "  "s);
    BOOST_TEST(UrlDecode("%2020"sv) == " 20"s);
    BOOST_TEST(UrlDecode("%20J"sv) == " J"s);
    BOOST_TEST(UrlDecode("Hello%20my%20name%20is%20 Dima"sv) == "Hello my name is  Dima"s);
}