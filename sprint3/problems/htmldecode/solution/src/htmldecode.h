#pragma once

#include <string>
#include <unordered_map>

const static std::unordered_map<std::string, std::string> HTML_MNEMONICS {
    {"lt", "<"},
    {"LT", "<"},

    {"gt", ">"},
    {"GT", ">"},

    {"amp", "&"},
    {"AMP", "&"},

    {"apos", "\'"},
    {"APOS", "\'"},

    {"quot", "\""},
    {"QUOT", "\""},
};

/*
 * Декодирует основные HTML-мнемоники:
 * - &lt - <
 * - &gt - >
 * - &amp - &
 * - &pos - '
 * - &quot - "
 *
 * Мнемоника может быть записана целиком либо строчными, либо заглавными буквами:
 * - &lt и &LT декодируются как <
 * - &Lt и &lT не мнемоники
 *
 * После мнемоники может стоять опциональный символ ;
 * - M&amp;M&APOSs декодируется в M&M's
 * - &amp;lt; декодируется в &lt;
 */
std::string HtmlDecode(std::string_view str);