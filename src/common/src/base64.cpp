#include "common/Base64.h"
#include <boost/beast/core/detail/base64.hpp>

namespace common::base64 {
std::string encode(std::span<const uint8_t> binary_data)
{
    std::span<const uint8_t> sp(binary_data.data(), binary_data.size());
    return encode(sp);
}

std::string encode(std::string_view binary_data)
{
    std::string base64;
    base64.resize(boost::beast::detail::base64::encoded_size(binary_data.size()));
    boost::beast::detail::base64::encode(&base64[0], binary_data.data(), binary_data.size());
    return std::move(base64);
}

std::string decode(std::string_view data)
{
    std::string base64;
    base64.resize(boost::beast::detail::base64::decoded_size(data.size()));
    boost::beast::detail::base64::decode(base64.data(), data.data(), data.size());
    return std::move(base64);
}

} // namespace common::base64