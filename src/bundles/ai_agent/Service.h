#pragma once
#include "cppmicroservices/BundleContext.h"
#include "imgui.h"
#include "service/IAIAgentService.h"
#include "service/ITaskService.h"
#include "trust_mode.h"
#include <boost/asio/io_context.hpp>
#include <httplib/client/client_pool.hpp>

class Config;

class Service
    : public service::ITaskService
    , public service::IAIAgentService
    , public std::enable_shared_from_this<Service>
{
  public:
    Service(cppmicroservices::BundleContext const& context, std::shared_ptr<ProgramTrustModeDrawer> trustMode);

  public:
    // ITaskService 接口实现
    std::string name() const override;
    std::string displayName() const override;

    bool onThreadRun() override;

    bool onThreadStart(std::shared_ptr<IBasicConfig> config) override;

    void onThreadEnd() override;

    std::shared_ptr<IBasicConfig> createConfig() const override;

    void requestStop() override;

    // IAIAgentService 接口实现
    std::shared_ptr<IDetectPanel> createDetectPanel() const override;
    void detect(std::shared_ptr<IDetectPanel> panel) override;
    boost::asio::awaitable<void> coroDetect(std::shared_ptr<IDetectPanel> panel) override;

    bool isTrustProgram(std::string const& line, std::string const& station, std::string const& name) override;

  private:
    httplib::client::http_client_pool::ClientHandle createHttpClient(
        std::optional<std::chrono::milliseconds> const& timeout_seconds);

  private:
    cppmicroservices::BundleContext bundleContext_;
    std::shared_ptr<ProgramTrustModeDrawer> trustMode_;

    cppmicroservices::ServiceRegistration<service::IAIAgentService> m_regAIAgentService;

    std::shared_ptr<Config> selfConfig_;
    std::unique_ptr<boost::asio::io_context> ioc_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> workGuard_;
    std::shared_ptr<httplib::client::http_client_pool> clientPool_;
};