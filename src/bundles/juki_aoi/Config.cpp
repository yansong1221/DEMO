#include "Config.h"

Config::Config() : m_aoiConfigs_([]() { return std::make_shared<AoiConfig>(); }) {}

std::vector<ImGui::extend::GroupBasicConfig::Item>
Config::prepareItems()
{
    return {
        { "aoi_configs", &m_aoiConfigs_ },
    };
}
