#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
    CHECK(HtmlDecode("hello"sv) == "hello"s);
    CHECK(HtmlDecode("&&&"sv) == "&&&"s);
    CHECK(HtmlDecode("Johnson&Johnson"sv) == "Johnson&Johnson"s);
}

TEST_CASE("Text with mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("M&amp;M&APOSs"sv) == "M&M's"s);
    CHECK(HtmlDecode("&amp;lt;"sv) == "&lt;"s);
    CHECK(HtmlDecode("Johnson&amp;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&ampJohnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AMP;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AMPJohnson"sv) == "Johnson&Johnson"s);
}

TEST_CASE("Text with mnemonics in the end", "[HtmlDecode]") {
    CHECK(HtmlDecode("Johnson&amp;"sv) == "Johnson&"s);
    CHECK(HtmlDecode("Johnson&amp"sv) == "Johnson&"s);
    CHECK(HtmlDecode("Johnson&AMP;"sv) == "Johnson&"s);
    CHECK(HtmlDecode("Johnson&AMP"sv) == "Johnson&"s);
    CHECK(HtmlDecode("Johnson&amp;;"sv) == "Johnson&;"s);
}

TEST_CASE("Text with all mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&amp;&gt&lt;"sv) == "&><"s);
    CHECK(HtmlDecode("&amp;&gt&lt;&quot;"sv) == "&><\""s);
    CHECK(HtmlDecode("&amp;&gt&lt;&quot;&apos"sv) == "&><\"\'"s);
    CHECK(HtmlDecode("&amp;&GT&lt;"sv) == "&><"s);
    CHECK(HtmlDecode("&amp;&gt&LT;&quot;"sv) == "&><\""s);
    CHECK(HtmlDecode("&AMP;&gt&lt;&QUOT;&APOS"sv) == "&><\"\'"s);
}

TEST_CASE("Text with unfinished mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&am&gt&lt;"sv) == "&am><"s);
    CHECK(HtmlDecode("&&amp&gt&lt;"sv) == "&&><"s);
    CHECK(HtmlDecode("&&&gt&lt;"sv) == "&&><"s);
}

TEST_CASE("Text with wrong mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&aMp&gt&lt;"sv) == "&aMp><"s);
    CHECK(HtmlDecode("&&aMp;&gt&lt;"sv) == "&&aMp;><"s);
    CHECK(HtmlDecode("&&&gT&lt;"sv) == "&&&gT<"s);
}

// Напишите недостающие тесты самостоятельно
