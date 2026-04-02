#include "FileDialogImpl.h"
namespace ImGui::extend
{

    void
    FileDialogImpl::open(std::string const& key)
    {
        auto dialog = createDialog(key);
        dialog->open();
    }

    bool
    FileDialogImpl::isCompleted(std::string const& key) const
    {
        if (auto dialog = findDialog(key); dialog)
        {
            return dialog->isCompleted();
        }
        return false;
    }

    void
    FileDialogImpl::close(std::string const& key)
    {
        if (auto dialog = findDialog(key); dialog)
        {
            dialog->close();
        }
    }

    std::string
    FileDialogImpl::selectedDirectory(std::string const& key) const
    {
        if (auto dialog = findDialog(key); dialog)
        {
            return dialog->selectedDirectory();
        }
        return {};
    }

    Dialog*
    FileDialogImpl::findDialog(std::string const& key) const
    {
        auto it = m_dialogs.find(key);
        if (it != m_dialogs.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    Dialog*
    FileDialogImpl::createDialog(std::string const& key)
    {
        if (auto dialog = findDialog(key); dialog)
        {
            return dialog;
        }

        auto dialog = std::make_unique<Dialog>();
        Dialog* dialogPtr = dialog.get();
        m_dialogs[key] = std::move(dialog);
        return dialogPtr;
    }
} // namespace ImGui::extend