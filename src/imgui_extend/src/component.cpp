#include "imgui_extend/component.h"
#include "FileDialogImpl.h"
#include <QCoreApplication>
#include <QFileDialog>

namespace ImGui::extend
{
    bool
    Button(char const* label, bool enabled /*= true*/, ImVec2 const& size /*= ImVec2(0, 0)*/)
    {
        if (enabled)
        {
            return ImGui::Button(label, size);
        }
        else
        {
            ImU32 const disabled_fg = IM_COL32_BLACK;
            ImU32 const disabled_bg = IM_COL32(64, 64, 64, 255);

            ImGui::PushStyleColor(ImGuiCol_Text, disabled_fg);
            ImGui::PushStyleColor(ImGuiCol_Button, disabled_bg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, disabled_bg);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, disabled_bg);
            ImGui::Button(label, size);
            ImGui::PopStyleColor(4);

            return false;
        }
    }

    bool
    DeleteButton(char const* label /*= "删除"*/, bool enabled /*= true*/, ImVec2 const& size /*= ImVec2(0, 0)*/)
    {
        auto displayLabel
            = label ? QByteArray(label) : QCoreApplication::translate("imgui::extend", "Delete").toUtf8();

        if (!enabled)
        {
            return Button(displayLabel, enabled, size);
        }

        ImVec4 buttonColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));

        bool clicked = ImGui::Button(displayLabel, size);
        ImGui::PopStyleColor(3);
        return clicked;
    }

    bool
    InputInt(char const* label,
             int* v,
             int v_min /*= std::numeric_limits<int>::min()*/,
             int v_max /*= std::numeric_limits<int>::max()*/,
             int step /*= 1*/,
             int step_fast /*= 100*/,
             ImGuiInputTextFlags flags /*= 0*/)
    {
        int temp = *v;
        if (ImGui::InputInt(label, &temp, step, step_fast, flags))
        {
            if (temp >= v_min && temp <= v_max)
            {
                *v = temp;
                return true;
            }
        }
        return false;
    }

    bool
    ToggleButton(char const* label, bool* v)
    {
        ImVec4 color;

        if (*v)
        {
            color = ImVec4(0.2f, 0.7f, 0.2f, 1.0f); // 开启时绿色
        }
        else
        {
            color = ImVec4(0.7f, 0.2f, 0.2f, 1.0f); // 关闭时红色
        }

        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

        bool clicked = ImGui::Button(label);

        if (clicked)
        {
            *v = !*v;
        }

        ImGui::PopStyleColor(3);

        return clicked;
    }
    FileDialog::FileDialog() : impl_(std::make_unique<FileDialogImpl>()) {}
    FileDialog::~FileDialog() {}

    bool
    FileDialog::InputDirectory(std::string const& key, std::string const& label, std::string* directory)
    {
        bool modified = false;

        ImGui::PushID(label.c_str());

        ImGui::BeginGroup();

        ImGui::TextUnformatted(label.c_str());
        if (ImGui::InputText("##", directory))
        {
            modified = true;
        }
        ImGui::SameLine();

        if (ImGui::Button(QCoreApplication::translate("imgui::extend", "Select").toUtf8()))
        {
            impl_->open(key);
        }
        if (impl_->isCompleted(key))
        {
            std::string selectedDir = impl_->selectedDirectory(key);
            if (!selectedDir.empty())
            {
                *directory = selectedDir;
                modified = true;
            }
            impl_->close(key);
        }
        ImGui::EndGroup();

        ImGui::PopID();
        return modified;
    }

    bool
    FileDialog::InputDirectory(std::string const& key, std::string const& label, std::filesystem::path* directory)
    {
        std::string u8_directory = QString::fromStdWString(directory->wstring()).toStdString();
        if (InputDirectory(key, label, &u8_directory))
        {
            *directory = std::filesystem::u8path(u8_directory);
            return true;
        }
        return false;
    }

} // namespace ImGui::extend