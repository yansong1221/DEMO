#pragma once
#include <boost/asio/awaitable.hpp>
#include <boost/asio/strand.hpp>

namespace common::coro {
class ticker : public std::enable_shared_from_this<ticker>
{
public:
    explicit ticker(
        const boost::asio::any_io_executor& executor,
        const std::chrono::steady_clock::duration& interval = std::chrono::milliseconds(500));
    virtual ~ticker();

public:
    virtual void start();
    virtual void stop();

    boost::asio::any_io_executor get_executor() const noexcept;

protected:
    virtual boost::asio::awaitable<bool> on_start() { co_return true; }
    virtual boost::asio::awaitable<bool> on_tick() = 0;
    virtual boost::asio::awaitable<void> on_stop() { co_return; }

protected:
    boost::asio::awaitable<boost::system::error_code> co_start();
    auto& get_strand() & { return strand_; }

private:
    boost::asio::awaitable<boost::system::error_code> co_run();

private:
    boost::asio::any_io_executor executor_;
    boost::asio::strand<boost::asio::any_io_executor> strand_;


    std::chrono::steady_clock::duration interval_;
    std::atomic<bool> is_running_ {false};

    boost::asio::cancellation_signal cs_;
};
} // namespace common::coro