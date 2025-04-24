#pragma once

#include <string_view>
#include <vector>

std::vector<const char *> to_c_strings(const std::vector<std::string_view> &strings);