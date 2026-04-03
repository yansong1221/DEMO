#pragma once
#include "Config.h"
#include "common/coro/ticker.h"
#include "cppmicroservices/ServiceTracker.h"
#include "service/IAIAgentService.h"
#include "service/ITaskService.h"
#include <boost/asio/io_context.hpp>

class Service : public service::ITaskService
{
  public:
    Service(cppmicroservices::BundleContext const& context);
    ~Service();

  public:
    std::string displayName() const override;

    std::string name() const override;

    bool onThreadRun() override;

    bool onThreadStart(std::shared_ptr<IBasicConfig> config) override;

    void onThreadEnd() override;

    std::shared_ptr<IBasicConfig> createConfig() const override;

    void requestStop() override;

  private:
    cppmicroservices::ServiceTracker<service::IAIAgentService> aiAgentTracker_;
    std::shared_ptr<Config> selfConfig_;
    std::vector<std::shared_ptr<common::coro::ticker>> tasks_;
    std::unique_ptr<boost::asio::io_context> ioc_;
};