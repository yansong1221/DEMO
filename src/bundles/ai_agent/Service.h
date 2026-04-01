#pragma once
#include "imgui.h"
#include "service/IAIAgentService.h"
#include "service/ITaskService.h"
#include <boost/asio/io_context.hpp>

class Service : public service::ITaskService,
                public service::IAIAgentService,
                public std::enable_shared_from_this<Service>
{
public:
    Service();

public:
    std::string name() const override;

    bool onThreadRun() override;

    bool onThreadStart(std::shared_ptr<IBasicConfig> config) override;

    void onThreadEnd() override;

    std::shared_ptr<IBasicConfig> createConfig() const override;

    void requestStop() override;

    // IAIAgentService 接口实现
    std::shared_ptr<IDetectPanel> createDetectPanel() const override;
    void detect(std::shared_ptr<IDetectPanel> panel) override;

private:
    std::unique_ptr<boost::asio::io_context> ioc_;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>
        workGuard_;
};