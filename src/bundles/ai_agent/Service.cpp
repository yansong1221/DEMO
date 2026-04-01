#include "Service.h"
#include "Config.h"
#include "DetectPanel.h"
#include <boost/asio/dispatch.hpp>

Service::Service() {}

std::string
Service::name() const
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
    return true;
}

void
Service::onThreadEnd()
{
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
Service::co_detect(std::shared_ptr<IDetectPanel> panel)
{
    co_return;
}