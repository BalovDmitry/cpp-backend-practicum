#include "htmldecode.h"

std::string HtmlDecode(std::string_view str) {
    const static int MAX_MNEMONIC_SIZE = 4;

    std::string result;
    result.reserve(str.size());

    std::string buffer;
    buffer.reserve(MAX_MNEMONIC_SIZE);

    bool possible_mnemonic = false;

    for (int i = 0; i < str.size(); ++i) {
        if (str[i] != '&' && !possible_mnemonic) {
            result.push_back(str[i]);
            continue;
        } else if (str[i] == '&' && possible_mnemonic) {
            result.push_back('&');
            if (!buffer.empty()) {
                result.append(buffer);
                buffer.clear();
            }
            continue;
        } else if (str[i] == '&') {
            possible_mnemonic = true;
            continue;
        }

        buffer.push_back(str[i]);
        if (buffer.size() <= MAX_MNEMONIC_SIZE) {
            //buffer.push_back(str[i]);
            if (HTML_MNEMONICS.contains(buffer)) {
                result.append(HTML_MNEMONICS.at(buffer));
                possible_mnemonic = false;
                buffer.clear();
                if (i < str.size() - 1 && str[i + 1] == ';') 
                    ++i;
            }
        } else {
            result.push_back('&');
            result.append(buffer);
            buffer.clear();
            possible_mnemonic = false;
        }
    }

    if (!buffer.empty())
        result.append(buffer);
    else if (buffer.empty() && possible_mnemonic)
        result.push_back('&');

    return result;
}
