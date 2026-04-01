#pragma once
#include <span>
#include <string>
#include <vector>

namespace common::base64
{

    std::string encode(std::span<uint8_t const> binary_data);
    std::string encode(std::string_view binary_data);
    std::string encode(std::vector<uint8_t> const& binary_data);
    std::string decode(std::string_view data);

} // namespace common::base64