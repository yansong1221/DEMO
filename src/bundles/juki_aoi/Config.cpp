#include "Config.h"

Config::Config()
{
    aoiConfigs.setCreateFunc<AoiConfig>();
}

std::vector<ImGui::extend::GroupBasicConfig::Item>
Config::prepareItems()
{
    return {
        { "aoi_configs", &aoiConfigs },
    };
}
