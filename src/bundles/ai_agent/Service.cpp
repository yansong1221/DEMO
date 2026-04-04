#include "Service.h"
#include "Config.h"
#include "DetectPanel.h"
#include "common/logger.h"
#include "common/misc.h"
#include <boost/asio/dispatch.hpp>

Service::Service(cppmicroservices::BundleContext const& context, std::shared_ptr<ProgramTrustModeDrawer> trustMode)
    : bundleContext_(context)
    , trustMode_(trustMode)
{
}

std::string
Service::name() const
{
    return "ai-agent";
}
std::string
Service::displayName() const
{
    return "ai-agent";
}
bool
Service::onThreadRun()
{
    ioc_->run();
    return false;
}

bool
Service::onThreadStart(std::shared_ptr<IBasicConfig> config)
{
    selfConfig_ = std::static_pointer_cast<Config>(config);

    ioc_ = std::make_unique<boost::asio::io_context>();
    workGuard_ = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
        boost::asio::make_work_guard(*ioc_));
    clientPool_
        = std::make_shared<httplib::client::http_client_pool>(ioc_->get_executor(), selfConfig_->ip, selfConfig_->port);

    m_regAIAgentService = bundleContext_.RegisterService<service::IAIAgentService>(shared_from_this());
    return true;
}

void
Service::onThreadEnd()
{
    m_regAIAgentService.Unregister();

    ioc_->stop();
    workGuard_.reset();
    selfConfig_.reset();
    ioc_.reset();
}

std::shared_ptr<service::ITaskService::IBasicConfig>
Service::createConfig() const
{
    return std::make_shared<Config>();
}

void
Service::requestStop()
{
    if (!ioc_)
    {
        return;
    }

    boost::asio::dispatch(ioc_->get_executor(), [this, self = shared_from_this()]() { workGuard_.reset(); });
}

std::shared_ptr<service::IAIAgentService::IDetectPanel>
Service::createDetectPanel() const
{
    return std::make_shared<DetectPanelImpl>();
}

void
Service::detect(std::shared_ptr<service::IAIAgentService::IDetectPanel> panel)
{
    // 暂时空实现
}
boost::asio::awaitable<void>
Service::coroDetect(std::shared_ptr<IDetectPanel> panel)
{
    common::Log::info("开始发送检测信息 LINE: {} STATION: {} NAME: {} SN: {} 给AI: [{}:{}]",
                      panel->line(),
                      panel->station(),
                      panel->name(),
                      panel->sn(),
                      selfConfig_->ip,
                      selfConfig_->port);
    try
    {
        auto panelImpl = std::static_pointer_cast<DetectPanelImpl>(panel);

        auto cli = createHttpClient(std::chrono::milliseconds(selfConfig_->detect_timeout_ms));

        auto response = co_await cli->async_post("/collect/sync", panelImpl->toDetectJson());
        if (!response)
        {
            common::Log::warn("LINE: {} STATION: {} NAME: {} SN: {} 给AI: {} 发送检测信息 错误: {}",
                              panelImpl->line(),
                              panelImpl->station(),
                              panelImpl->name(),
                              panelImpl->sn(),
                              cli->host(),
                              common::misc::to_u8string(response.error().message()));
            co_return;
        }
        if (panelImpl->isTrustModeEnabled())
        {
            panelImpl->parseAiResult(response->body().as<httplib::body::json_body>());
        }
    }
    catch (std::exception const& e)
    {
        common::Log::warn("LINE: {} STATION: {} NAME: {} SN: {} 发送检测信息 错误: {}",
                          panel->line(),
                          panel->station(),
                          panel->name(),
                          panel->sn(),
                          common::misc::to_u8string(e.what()));
    }
    common::Log::info("结束发送检测信息 LINE: {} STATION: {} NAME: {} SN: {} 给AI: [{}:{}]",
                      panel->line(),
                      panel->station(),
                      panel->name(),
                      panel->sn(),
                      selfConfig_->ip,
                      selfConfig_->port);
    co_return;
}

bool
Service::isTrustProgram(std::string const& line, std::string const& station, std::string const& name)
{
    return trustMode_->isTrust(line, station, name);
}

httplib::client::http_client_pool::ClientHandle
Service::createHttpClient(std::optional<std::chrono::milliseconds> const& timeout_seconds)
{
    auto cli = clientPool_->acquire();
    if (timeout_seconds)
    {
        cli->set_timeout(*timeout_seconds);
        cli->set_timeout_policy(httplib::client::http_client::timeout_policy::overall);
    }
    else
    {
        cli->set_timeout_policy(httplib::client::http_client::timeout_policy::never);
    }
    return cli;
}
