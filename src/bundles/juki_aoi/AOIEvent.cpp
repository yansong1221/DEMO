#include "AOIEvent.h"
#include "Panel.h"
#include "common/SafeFile.h"
#include "common/logger.h"

AOIEvent::AOIEvent(boost::asio::any_io_executor const& executor,
                   cppmicroservices::ServiceTracker<service::IAIAgentService> const& tracker,
                   std::shared_ptr<Config::AoiConfig> const& conf)
    : ticker(executor)
    , tracker_(tracker)
    , conf_(conf)
{
}

boost::asio::awaitable<bool>
AOIEvent::on_tick()
{
    auto aiAgentSer = tracker_.GetService();
    if (!aiAgentSer)
    {
        common::Log::error("无法获取AI服务");
        co_return true;
    }
    aiAgentSer->isTrustProgram("1", "2", "3");

    auto xml1 = std::filesystem::u8path(conf_->aoi_dir) / "xml1";

    for (auto const& xmlPath : common::SafeFile::walkFiles(xml1, { ".xml" }, true))
    {
        co_await procAoiXMLFile(xmlPath);
    }
    co_return true;
}

boost::asio::awaitable<bool>
AOIEvent::procAoiXMLFile(std::filesystem::path const& xmlPath)
{
    try
    {
        auto aiAgentSer = tracker_.GetService();
        if (!aiAgentSer)
        {
            common::Log::error("无法获取AI服务");
            co_return false;
        }
        auto panel = Panel::loadFromFile(xmlPath, aiAgentSer->createDetectPanel());
        if (!panel)
        {
            co_return false;
        }
        co_await aiAgentSer->coroDetect(panel->detectPanel);

        co_return true;
    }
    catch (std::exception const& e)
    {
        common::Log::error("处理AOI输出的文件: {}, 异常: {}",
                           common::misc::to_u8string(xmlPath),
                           common::misc::to_u8string(e.what()));
    }
    co_return false;
}
