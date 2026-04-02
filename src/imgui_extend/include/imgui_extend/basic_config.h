#pragma once
#include "export.h"
#include "service/ITaskService.h"
#include <functional>
#include <vector>

namespace ImGui::extend
{
    class IMGUI_EXTEND_API BasicConfig : public service::ITaskService::IBasicConfig
    {
      public:
        virtual std::string
        displayName() const
        {
            return {};
        }
    };

    class IMGUI_EXTEND_API ArrayBasicConfig : virtual public BasicConfig
    {
        using BasicConfigPtr = std::shared_ptr<BasicConfig>;
        using CreateFunc = std::function<BasicConfigPtr()>;

      public:
        ArrayBasicConfig(CreateFunc&& func) : m_createFunc(std::move(func)) {}

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

      private:
        CreateFunc m_createFunc;
        std::vector<BasicConfigPtr> m_items;
    };

    class IMGUI_EXTEND_API GroupBasicConfig : virtual public BasicConfig
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

            Item(std::string const& _key, BasicConfig* _conf, bool _visable = true, ViewType _view = ViewType::tab)
                : key(_key)
                , conf(_conf)
                , visable(_visable)
                , view(_view)
            {
            }

            std::string key;
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

} // namespace ImGui::extend
