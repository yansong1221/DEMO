#pragma once
#include "export.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <filesystem>
#include <limits>

namespace ImGui::extend
{
    IMGUI_EXTEND_API bool Button(char const* label, bool enabled = true, ImVec2 const& size = ImVec2(0, 0));

    IMGUI_EXTEND_API bool DeleteButton(char const* label = nullptr,
                                       bool enabled = true,
                                       ImVec2 const& size = ImVec2(0, 0));

    IMGUI_EXTEND_API bool InputInt(char const* label,
                                   int* v,
                                   int v_min = std::numeric_limits<int>::min(),
                                   int v_max = std::numeric_limits<int>::max(),
                                   int step = 1,
                                   int step_fast = 100,
                                   ImGuiInputTextFlags flags = 0);

    template <typename T>
    void
    BeginChild(T&& callback,
               char const* str_id,
               ImVec2 const& size = ImVec2(0, 0),
               ImGuiChildFlags child_flags = 0,
               ImGuiWindowFlags window_flags = 0)
    {
        if (ImGui::BeginChild(str_id, size, child_flags, window_flags))
        {
            callback();
        }
        ImGui::EndChild();
    }
    IMGUI_EXTEND_API bool ToggleButton(char const* label, bool* v);

    class FileDialogImpl;
    class IMGUI_EXTEND_API FileDialog
    {
      public:
        FileDialog();
        virtual ~FileDialog();

      public:
        bool InputDirectory(std::string const& key, std::string const& label, std::string* directory);
        bool InputDirectory(std::string const& key, std::string const& label, std::filesystem::path* directory);

      private:
        std::unique_ptr<FileDialogImpl> impl_;
    };

} // namespace ImGui::extend