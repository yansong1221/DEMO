#include "imgui_extend/component.h"
#include "ImGuiFileDialog.h"
#include <QCoreApplication>
#include <QFileDialog>

namespace ImGui::extend
{
    namespace detail
    {
        static std::string
        translate(char const* key)
        {
            return QCoreApplication::translate("common::ui::component", key).toStdString();
        }
    } // namespace detail

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
    DeteleButton(char const* label /*= "删除"*/, bool enabled /*= true*/, ImVec2 const& size /*= ImVec2(0, 0)*/)
    {

        if (!enabled)
        {
            return Button(label ? label : detail::translate("Delete").c_str(), enabled, size);
        }

        ImVec4 buttonColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));

        bool clicked = ImGui::Button(label ? label : detail::translate("Delete").c_str(), size);
        ImGui::PopStyleColor(3);
        return clicked;
    }

    // bool
    // InputDirectory(std::string const& label, std::filesystem::path* directory)
    //{
    //     std::string u8_directory = misc::to_u8string(directory->string());
    //     if (InputDirectory(label, &u8_directory))
    //     {
    //         *directory = std::filesystem::u8path(u8_directory);
    //         return true;
    //     }
    //     return false;
    // }

    bool
    InputDirectory(std::string const& label, std::string* directory)
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

        if (ImGui::Button(QCoreApplication::translate("common::ui::component", "Select").toUtf8()))
        {
            // auto dlg = new QFileDialog();
            // dlg->setAttribute(Qt::WA_DeleteOnClose);
            // QObject::connect(dlg, &QFileDialog::fileSelected, dlg, [dlg](QString const& file) { modified = true; });
            // dlg->open();

            // QString file = QFileDialog::getOpenFileName(nullptr, "Open File", "", "All Files (*.*)");

            // ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

            //// IGFD::FileDialogConfig config;
            //// config.path = IGFD::FileDialog::Instance()->GetDrivesList();
            // IGFD::FileDialogConfig config;
            // config.path = ".";
            // ImGuiFileDialog::Instance()->OpenDialog(key, "Choose File", nullptr, config);
        }
        // if (ImGuiFileDialog::Instance()->Display(key))
        //{
        //     if (ImGuiFileDialog::Instance()->IsOk())
        //     {
        //         *directory = ImGuiFileDialog::Instance()->GetCurrentPath();
        //         modified = true;
        //     }

        //    // close
        //   ImGuiFileDialog::Instance()->Close();
        //}

        ImGui::EndGroup();

        ImGui::PopID();
        return modified;
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

    class FileDialogImpl
    {
      public:
        QFileDialog dialog;
        QString selectedDirectory;
        bool completed = false;
    };

    FileDialog::FileDialog() : impl_(new FileDialogImpl())
    {
        impl_->dialog.setFileMode(QFileDialog::Directory);
        impl_->dialog.setOption(QFileDialog::Option::ShowDirsOnly);

        QObject::connect(&impl_->dialog,
                         &QFileDialog::fileSelected,
                         &impl_->dialog,
                         [this](QString const& file)
                         {
                             impl_->selectedDirectory = file;
                             impl_->completed = true;
                         });
    }
    FileDialog::~FileDialog() {}

    void
    FileDialog::open()
    {
        impl_->dialog.open();
    }

    std::string
    FileDialog::selectedDirectory() const
    {
        return impl_->selectedDirectory.toStdString();
    }

    bool
    FileDialog::display()
    {
        if (impl_->completed)
        {
            return true;
        }
        return false;
    }

    void
    FileDialog::close()
    {
        impl_->completed = false;
        impl_->dialog.close();
    }

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

        if (ImGui::Button(QCoreApplication::translate("common::ui::component", "Select").toUtf8()))
        {
            // auto dlg = new QFileDialog();
            // dlg->setAttribute(Qt::WA_DeleteOnClose);
            // QObject::connect(dlg, &QFileDialog::fileSelected, dlg, [dlg](QString const& file) { modified = true; });
            // dlg->open();

            // QString file = QFileDialog::getOpenFileName(nullptr, "Open File", "", "All Files (*.*)");

            // ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

            //// IGFD::FileDialogConfig config;
            //// config.path = IGFD::FileDialog::Instance()->GetDrivesList();
            // IGFD::FileDialogConfig config;
            // config.path = ".";
            // ImGuiFileDialog::Instance()->OpenDialog(key, "Choose File", nullptr, config);
        }
        // if (ImGuiFileDialog::Instance()->Display(key))
        //{
        //     if (ImGuiFileDialog::Instance()->IsOk())
        //     {
        //         *directory = ImGuiFileDialog::Instance()->GetCurrentPath();
        //         modified = true;
        //     }

        //    // close
        //   ImGuiFileDialog::Instance()->Close();
        //}

        ImGui::EndGroup();

        ImGui::PopID();
        return modified;
    }

} // namespace ImGui::extend