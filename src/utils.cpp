#include "utils.h"

std::vector<const char *> to_c_strings(const std::vector<std::string_view> &strings) {
    std::vector<const char *> c_strings;
    c_strings.reserve(strings.size());
    for (auto &string : strings) {
        c_strings.push_back(string.data());
    }
    return c_strings;
}
