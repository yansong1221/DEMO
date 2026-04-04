#pragma once
#include "export.h"
#include "service/IAIAgentService.h"
#include "service/ITaskService.h"
#include "table_view.h"
#include <functional>
#include <vector>

namespace ImGui::extend
{
    class IMGUI_EXTEND_API BasicConfig : public service::ITaskService::IBasicConfig
    {
      public:
        virtual std::string displayName() const = 0;
        virtual std::string configKey() const = 0;
    };
    class IMGUI_EXTEND_API TrustModeConfig : public BasicConfig
    {
      public:
        enum class mode
        {
            off = 0,
            on,
            with_program
        };

        bool isTrust(std::shared_ptr<service::IAIAgentService> serv,
                     std::string const& line,
                     std::string const& station,
                     std::string const& program) const;

        std::string displayName() const override;
        std::string configKey() const override;

      public:
        void draw() override;
        void restore(YAML::Node item) override;
        void save(YAML::Node& conf) const override;

      private:
        mode trust_mode_ = mode::off;
    };

    class IMGUI_EXTEND_API ArrayBasicConfig : public BasicConfig
    {

      public:
        void draw() override;
        void save(YAML::Node& conf) const override;
        void restore(YAML::Node conf) override;

        template <typename T>
        std::vector<std::shared_ptr<T>>
        items() const
        {
            std::vector<std::shared_ptr<T>> result;
            for (auto const& item : m_items)
            {
                if (auto ptr = std::dynamic_pointer_cast<T>(item))
                {
                    result.push_back(ptr);
                }
            }
            return result;
        }

      protected:
        using BasicConfigPtr = std::shared_ptr<BasicConfig>;
        virtual BasicConfigPtr newConfigItem() = 0;

      protected:
        std::vector<BasicConfigPtr> m_items;
    };

    class IMGUI_EXTEND_API GroupBasicConfig : public BasicConfig
    {
      public:
        struct Item
        {
            enum ViewType
            {
                none,
                tab,
                tree,
            };

            Item(BasicConfig* _conf, bool _visable = true, ViewType _view = ViewType::tab)
                : conf(_conf)
                , visable(_visable)
                , view(_view)
            {
            }
            BasicConfig* conf;
            ViewType view;
            bool visable;
        };

      public:
        void draw() override;
        void save(YAML::Node& conf) const override;
        void restore(YAML::Node conf) override;

      protected:
        std::vector<Item>
        prepareItems() const
        {
            return const_cast<GroupBasicConfig*>(this)->prepareItems();
        }
        virtual std::vector<Item> prepareItems() = 0;
    };

    struct TableArrayBasicConfig
        : public TableView
        , public ArrayBasicConfig
    {
        void draw() override;
        void restore(YAML::Node item) override;

      protected:
        int columnCount() const override;

        std::string headerLable(int column) const override;
        int rowCount() const override;
        void drawCell(int row, int column) override;

        ImGuiTableColumnFlags headerFlags(int column) const override;
    };

    class IMGUI_EXTEND_API CustomAngleConfig : public GroupBasicConfig
    {
      public:
        std::optional<int> fixedAngleWithOriginalAngle(std::string_view line,
                                                       std::string_view station,
                                                       std::string_view program,
                                                       int board_index,
                                                       int original_angle) const;

        std::optional<int> fixedAngle(std::string_view line,
                                      std::string_view station,
                                      std::string_view program,
                                      int board_index) const;
        bool isRequired() const;

        std::string displayName() const override;
        std::string configKey() const override;

      protected:
        std::vector<GroupBasicConfig::Item> prepareItems() override;

        void draw() override;
        void restore(YAML::Node item) override;
        void save(YAML::Node& conf) const override;

      private:
        struct BoardInfoConfig : public BasicConfig
        {
            int board_index = 0;
            int angle = 0;

            void draw() override;
            void save(YAML::Node& conf) const override;
            void restore(YAML::Node item) override;
            std::string displayName() const override;
            std::string configKey() const override;
        };
        class BoardInfoArrayConfig : public TableArrayBasicConfig
        {
          public:
            std::string displayName() const override;
            std::string configKey() const override;
            BasicConfigPtr
            newConfigItem() override
            {
                return std::make_shared<BoardInfoConfig>();
            }
        };

        struct StationAngleConfig : public GroupBasicConfig
        {
            std::string line;
            std::string station;
            BoardInfoArrayConfig boards;

            std::vector<GroupBasicConfig::Item> prepareItems() override;

            void draw() override;
            void restore(YAML::Node item) override;
            void save(YAML::Node& conf) const override;
            std::string displayName() const override;
            std::string configKey() const override;
        };

        struct ProgramAngleConfig : public GroupBasicConfig
        {
            std::string name;
            BoardInfoArrayConfig boards;

            std::vector<GroupBasicConfig::Item> prepareItems() override;
            std::string displayName() const override;
            std::string configKey() const override;
            void draw() override;
            void restore(YAML::Node item) override;
            void save(YAML::Node& conf) const override;
        };

        class StationArrayBasicConfig : public ArrayBasicConfig
        {
          public:
            std::string displayName() const override;
            std::string configKey() const override;
            BasicConfigPtr
            newConfigItem() override
            {
                return std::make_shared<StationAngleConfig>();
            }
        };

        class ProgramArrayBasicConfig : public ArrayBasicConfig
        {
          public:
            std::string displayName() const override;
            std::string configKey() const override;
            BasicConfigPtr
            newConfigItem() override
            {
                return std::make_shared<ProgramAngleConfig>();
            }
        };

        bool required_ = false;
        StationArrayBasicConfig station_list_;
        ProgramArrayBasicConfig program_list_;
    };

} // namespace ImGui::extend
