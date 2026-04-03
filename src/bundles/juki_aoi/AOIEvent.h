#pragma once
#include "Config.h"
#include "common/coro/ticker.h"
#include "cppmicroservices/ServiceTracker.h"
#include "service/IAIAgentService.h"
#include <filesystem>

class AOIEvent : public common::coro::ticker
{
  public:
    AOIEvent(boost::asio::any_io_executor const& executor,
             cppmicroservices::ServiceTracker<service::IAIAgentService> const& tracker,
             std::shared_ptr<Config::AoiConfig> const& conf);

  protected:
    boost::asio::awaitable<bool> on_tick() override;

  private:
    boost::asio::awaitable<bool> procAoiXMLFile(std::filesystem::path const& xmlPath);

  private:
    cppmicroservices::ServiceTracker<service::IAIAgentService> const& tracker_;
    std::shared_ptr<Config::AoiConfig> conf_;
};