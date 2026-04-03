#include "imgui_extend/basic_config.h"
#include "imgui.h"
#include "imgui_extend/component.h"
#include <QCoreApplication>
#include <ranges>

namespace ImGui::extend
{
    void
    ArrayBasicConfig::draw()
    {
        for (int i = 0; i < m_items.size(); i++)
        {
            auto& info = m_items[i];
            auto d_name = info->displayName();
            auto label = QCoreApplication::translate("imgui::extend", "Config%1").arg(i + 1);

            ImGui::PushID(i);

            if (!d_name.empty())
            {
                label += QString::fromStdString(std::format("({})", d_name));
            }
            if (ImGui::TreeNodeEx("config", ImGuiTreeNodeFlags_DefaultOpen, label.toUtf8()))
            {
                ImGui::BeginGroup();
                info->draw();
                ImGui::EndGroup();

                ImGui::Separator();

                if (ImGui::extend::DeteleButton(
                        QCoreApplication::translate("imgui::extend", "Delete-[%1]").arg(label).toUtf8()))
                {
                    m_items.erase(m_items.begin() + i);
                    i--;
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            ImGui::PopID();
        }
        if (ImGui::Button(QCoreApplication::translate("imgui::extend", "Add").toUtf8()))
        {
            m_items.push_back(m_createFunc());
        }
    }

    void
    ArrayBasicConfig::save(YAML::Node& conf) const
    {
        conf.reset();
        for (auto const& confPtr : m_items)
        {
            YAML::Node yaml;
            confPtr->save(yaml);
            conf.push_back(yaml);
        }
    }

    void
    ArrayBasicConfig::restore(YAML::Node conf)
    {
        m_items.clear();
        for (auto item : conf)
        {
            auto confPtr = m_createFunc();
            confPtr->restore(item);
            m_items.push_back(confPtr);
        }
    }

    void
    GroupBasicConfig::draw()
    {
        auto items = prepareItems();
        auto visable_items = items | std::views::filter([](Item const& v) { return v.visable; });

        ImGui::PushID(this);

        int index = 0;
        for (auto& item : visable_items | std::views::filter([](Item const& v) { return v.view != Item::tab; }))
        {
            ImGui::PushID(index++);
            switch (item.view)
            {
                case Item::tree:
                {
                    if (ImGui::TreeNodeEx(item.conf->displayName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        item.conf->draw();
                        ImGui::TreePop();
                    }
                }
                break;
                default:
                {
                    ImGui::SeparatorText(item.conf->displayName().c_str());
                    ImGui::Indent();
                    item.conf->draw();
                    ImGui::Unindent();
                }
                break;
            }
            ImGui::PopID();
        }

        if (std::ranges::find_if(visable_items, [](Item const& v) { return v.view == Item::tab; })
            != visable_items.end())
        {
            ImGui::BeginTabBar("##TAB");
            for (auto& item : visable_items | std::views::filter([](Item const& v) { return v.view == Item::tab; }))
            {
                ImGui::PushID(index++);

                if (ImGui::BeginTabItem(item.conf->displayName().c_str()))
                {
                    item.conf->draw();
                    ImGui::EndTabItem();
                }

                ImGui::PopID();
            }
            ImGui::EndTabBar();
        }
        ImGui::PopID();
    }

    void
    GroupBasicConfig::save(YAML::Node& conf) const
    {
        for (auto const& item : prepareItems())
        {
            YAML::Node yaml;
            item.conf->save(yaml);
            conf[item.key] = yaml;
        }
    }

    void
    GroupBasicConfig::restore(YAML::Node conf)
    {
        for (auto& item : prepareItems())
        {
            YAML::Node yaml = conf[item.key];
            item.conf->restore(yaml);
        }
    }

} // namespace ImGui::extend