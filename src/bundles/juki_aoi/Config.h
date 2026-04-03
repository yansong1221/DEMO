#pragma once
#include "common/misc.h"
#include "imgui_extend/basic_config.h"
#include "imgui_extend/component.h"
#include "service/ITaskService.h"
#include <filesystem>

class Config : public ImGui::extend::GroupBasicConfig
{
  public:
    class AoiConfig : public ImGui::extend::BasicConfig
    {
      public:
        std::string
        displayName() const override
        {
            return "AOI配置";
        }
        void
        draw() override
        {
            fileDialog.InputDirectory("123", "AOI Directory", &aoi_dir);
        }
        void
        save(YAML::Node& conf) const override
        {
            conf["aoi_dir"] = aoi_dir;
        }
        void
        restore(YAML::Node conf) override
        {
            using namespace std::filesystem;
            aoi_dir = conf["aoi_dir"].as<std::string>("");
        }
        std::string aoi_dir;

      private:
        ImGui::extend::FileDialog fileDialog;
    };
    Config();

  protected:
    std::vector<Item> prepareItems() override;

  private:
    class AoiConfigs : public ImGui::extend::ArrayBasicConfig
    {
        using ArrayBasicConfig::ArrayBasicConfig;

      public:
        std::string
        displayName() const override
        {
            return "AOI Configs";
        }
    };

  public:
    AoiConfigs aoiConfigs;
};
