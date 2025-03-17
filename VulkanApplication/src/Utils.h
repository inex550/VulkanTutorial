#pragma once

#include <vector>
#include <string_view>

namespace nex {

namespace utils {

std::vector<char> ReadFile(std::string_view filepath);

} // namespace utils

} // namespace nex