#include "Panel.h"
#include "common/Logger.h"
#include "common/misc.h"
#include <pugixml.hpp>

std::shared_ptr<Panel>
Panel::loadFromFile(std::filesystem::path const& xmlPath, std::shared_ptr<service::IAIAgentService::IDetectPanel> panel)
{
    try
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(xmlPath.c_str());
        if (!result)
        {
            common::Log::error("打开文件失败:{} what:{}", common::misc::to_u8string(xmlPath), result.description());
            return nullptr;
        }

        auto PanelXml = doc.child("PanelXml");

        panel->setLine(PanelXml.child("StationId").text().as_string());
        panel->setStation(PanelXml.child("MachineName").text().as_string());
        panel->setName(PanelXml.child("ProjectName").text().as_string());
        panel->setSn(PanelXml.child("SerialNumOfTest").text().as_string());

        auto internalPanel = std::make_shared<Panel>();
        internalPanel->detectPanel = panel;
        return internalPanel;
    }
    catch (std::exception const& e)
    {
        common::Log::error("读取文件: {} 出现异常: {}",
                           common::misc::to_u8string(xmlPath),
                           common::misc::to_u8string(e.what()));
    }
    return nullptr;
}
