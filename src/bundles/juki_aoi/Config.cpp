#include "Config.h"

std::string
Config::displayName() const
{
    return {};
}

std::string
Config::configKey() const
{
    return {};
}

std::vector<ImGui::extend::GroupBasicConfig::Item>
Config::prepareItems()
{
    return {
        { &aoiConfigs },
        { &customAngle },
    };
}

std::string
Config::AoiConfigs::displayName() const
{

    return "AOI Configs";
}
