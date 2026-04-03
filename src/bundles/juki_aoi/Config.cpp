#include "Config.h"

Config::Config() : aoiConfigs([]() { return std::make_shared<AoiConfig>(); }) {}

std::vector<ImGui::extend::GroupBasicConfig::Item>
Config::prepareItems()
{
    return {
        { "aoi_configs", &aoiConfigs },
    };
}
