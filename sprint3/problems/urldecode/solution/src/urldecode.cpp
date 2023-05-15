#include "urldecode.h"

#include <charconv>
#include <stdexcept>

std::string UrlDecode(std::string_view str) {
    std::string ret;
    char ch;
    int temp;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] == '+') {
            ret += ' ';
        } else if (str[i] == '%') {
            sscanf(str.substr(i + 1, 2).data(), "%x", &temp);
            ch = static_cast<char>(temp);
            ret += ch;
            i += 2;
        } else {
            ret += str[i];
        }
    }
    return ret;
}
