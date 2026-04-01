#pragma once
#include <span>
#include <string>
#include <vector>

namespace common::base64 {

std::string encode(std::span<const uint8_t> binary_data);
std::string encode(std::string_view binary_data);
std::string encode(const std::vector<uint8_t>& binary_data);
std::string decode(std::string_view data);

} // namespace common::base64