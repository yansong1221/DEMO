#pragma once
#include "use_awaitable.hpp"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>

namespace common::coro {
inline static boost::asio::awaitable<boost::system::error_code>
sleep(boost::asio::steady_timer& timer, const std::chrono::steady_clock::duration& duration)
{
    timer.expires_after(duration);
    boost::system::error_code ec;
    co_await timer.async_wait(net_awaitable[ec]);
    co_return ec;
}

inline static boost::asio::awaitable<boost::system::error_code>
sleep(const std::chrono::steady_clock::duration& duration)
{
    boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
    co_return co_await sleep(timer, duration);
}
} // namespace common::coro