#include "common/Base64.h"
#include <boost/beast/core/detail/base64.hpp>

namespace common::base64
{
    std::string
    encode(std::span<uint8_t const> binary_data)
    {
        std::string _encode_content;
        _encode_content.resize(boost::beast::detail::base64::encoded_size(binary_data.size()));
        boost::beast::detail::base64::encode(&_encode_content[0], binary_data.data(), binary_data.size());
        return _encode_content;
    }

    std::string
    encode(std::string_view binary_data)
    {
        std::span<uint8_t const> sp((uint8_t const*)binary_data.data(), binary_data.size());
        return encode(sp);
    }

    std::string
    encode(std::vector<uint8_t> const& binary_data)
    {
        std::span<uint8_t const> sp(binary_data.data(), binary_data.size());
        return encode(sp);
    }

    std::string
    decode(std::string_view data)
    {
        std::string _decode_content;
        _decode_content.resize(boost::beast::detail::base64::decoded_size(data.size()));
        boost::beast::detail::base64::decode(_decode_content.data(), data.data(), data.size());
        return _decode_content;
    }

} // namespace common::base64