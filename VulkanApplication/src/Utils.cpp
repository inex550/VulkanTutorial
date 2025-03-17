#include "Utils.h"

#include <fstream>
#include <stdexcept>

namespace nex {

namespace utils {

std::vector<char> ReadFile(std::string_view filepath) {
    std::vector<char> content;

    std::ifstream file(filepath.data(), std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("File for read does not exists");
    }

    size_t fileSize = file.tellg();
    content.resize(fileSize);

    file.seekg(file.beg);
    file.read(content.data(), content.size());

    return content;
}

} // namespace utils

} // namespace nex