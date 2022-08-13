#include "Helper.h"

#include <algorithm>

// FIXME do these work with unicode?

bool strContains(const std::string& s1, const std::string& s2) {
    return s1.find(s2) != std::string::npos;
}

std::string strDropQuotes(const std::string& s) {
    return s.size() >= 2 && strStartsWith(s, "'") && strEndsWith(s, "'")
        ? s.substr(1, s.size() - 2)
        : s;
}

/* see: https://stackoverflow.com/a/42844629 */
bool strEndsWith(const std::string& s1, const std::string& s2) {
    return s1.size() >= s2.size() && s1.compare(s1.size() - s2.size(), s2.size(), s2) == 0;
}

bool strEquals(const std::string& s1, const std::string& s2) {
    return s1.compare(s2) == 0;
}

/* see: https://stackoverflow.com/a/4654718 */
bool strIsNumeric(const std::string& s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

/* see: https://stackoverflow.com/a/3418285 */
void strReplace(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

/* see: https://stackoverflow.com/a/42844629 */
bool strStartsWith(const std::string& s1, const std::string& s2) {
    return s1.size() >= s2.size() && s1.compare(0, s2.size(), s2) == 0;
}
