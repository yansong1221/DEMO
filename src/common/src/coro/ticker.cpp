#include "common/coro/ticker.h"
#include "common/coro/sleep.hpp"
#include "common/misc.h"
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>
#include <spdlog/spdlog.h>

namespace common::coro
{
    ticker::ticker(boost::asio::any_io_executor const& executor, std::chrono::steady_clock::duration const& interval)
        : executor_(executor)
        , strand_(executor)
        , interval_(interval)
    {
    }

    ticker::~ticker() {}

    void
    ticker::start()
    {
        boost::asio::co_spawn(
            strand_,
            [this, self = shared_from_this()]() -> boost::asio::awaitable<void>
            {
                if (is_running_)
                {
                    co_return;
                }

                co_await co_start();
            },
            boost::asio::bind_cancellation_slot(cs_.slot(),
                                                [](std::exception_ptr e)
                                                {
                                                    if (e)
                                                    {
                                                        std::rethrow_exception(e);
                                                    }
                                                }));
    }

    void
    ticker::stop()
    {
        boost::asio::dispatch(strand_,
                              [this, self = shared_from_this()]()
                              {
                                  if (!is_running_)
                                  {
                                      return;
                                  }
                                  cs_.emit(boost::asio::cancellation_type::all);
                              });
    }

    boost::asio::any_io_executor
    ticker::get_executor() const noexcept
    {
        return executor_;
    }

    boost::asio::awaitable<boost::system::error_code>
    ticker::co_start()
    {
        co_await boost::asio::dispatch(strand_);

        boost::system::error_code ec;
        if (!is_running_)
        {
            is_running_ = true;
            ec = co_await co_run();
            is_running_ = false;
        }
        co_return ec;
    }

    boost::asio::awaitable<boost::system::error_code>
    ticker::co_run()
    {
        co_await boost::asio::this_coro::reset_cancellation_state(boost::asio::enable_total_cancellation(),
                                                                  boost::asio::enable_terminal_cancellation());

        co_await boost::asio::this_coro::throw_if_cancelled(false);

        auto cs = co_await boost::asio::this_coro::cancellation_state;

        try
        {
            if (!co_await on_start())
            {
                co_return boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
            }
        }
        catch (boost::system::system_error const& e)
        {
            spdlog::error("执行协程开始出现异常: {}", misc::to_u8string(e.code().message()));
            co_return e.code();
        }
        catch (std::exception const& e)
        {
            spdlog::error("执行协程开始出现异常: {}", misc::to_u8string(e.what()));
            co_return boost::system::errc::make_error_code(boost::system::errc::interrupted);
        }
        catch (...)
        {
            spdlog::error("执行协程开始出现未知异常");
            co_return boost::system::errc::make_error_code(boost::system::errc::interrupted);
        }

        boost::system::error_code ec;
        boost::asio::steady_timer update_timer(co_await boost::asio::this_coro::executor);
        for (; !ec; ec = co_await sleep(update_timer, interval_))
        {
            try
            {
                if (!co_await on_tick())
                {
                    break;
                }
            }
            catch (boost::system::system_error const& e)
            {
                spdlog::error("执行协程任务出现异常: {}", misc::to_u8string(e.code().message()));
                ec = e.code();
                break;
            }
            catch (std::exception const& e)
            {
                spdlog::error("执行协程任务出现异常: {}", misc::to_u8string(e.what()));
                ec = boost::system::errc::make_error_code(boost::system::errc::interrupted);
                break;
            }
            catch (...)
            {
                spdlog::error("执行协程任务出现未知异常");
                ec = boost::system::errc::make_error_code(boost::system::errc::interrupted);
                break;
            }

            if (!!cs.cancelled())
            {
                break;
            }
        }

        try
        {
            co_await on_stop();
        }
        catch (boost::system::system_error const& e)
        {
            spdlog::error("执行协程结束出现异常: {}", misc::to_u8string(e.code().message()));
            if (!ec)
            {
                ec = e.code();
            }
        }
        catch (std::exception const& e)
        {
            spdlog::error("执行协程结束出现异常: {}", misc::to_u8string(e.what()));
            if (!ec)
            {
                ec = boost::system::errc::make_error_code(boost::system::errc::interrupted);
            }
        }
        catch (...)
        {
            spdlog::error("执行协程结束出现未知异常");
            if (!ec)
            {
                ec = boost::system::errc::make_error_code(boost::system::errc::interrupted);
            }
        }
        co_return ec;
    }
} // namespace common::coro