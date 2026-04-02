#pragma once
#include <QFileDialog>
namespace ImGui::extend
{

    class Dialog
    {
      public:
        Dialog()
        {
            m_dialog.setFileMode(QFileDialog::Directory);
            m_dialog.setOption(QFileDialog::Option::ShowDirsOnly);

            QObject::connect(&m_dialog,
                             &QFileDialog::fileSelected,
                             &m_dialog,
                             [this](QString const& file)
                             {
                                 m_selectedDirectory = file.toStdString();
                                 m_completed = true;
                             });
        }
        void
        open()
        {
            m_dialog.open();
        }
        bool
        isCompleted() const
        {
            return m_completed;
        }
        void
        close()
        {
            m_dialog.close();
            m_completed = false;
        }
        std::string const&
        selectedDirectory() const
        {
            return m_selectedDirectory;
        }

      private:
        QFileDialog m_dialog;
        std::string m_selectedDirectory;
        bool m_completed = false;
    };

    class FileDialogImpl
    {
      public:
        void open(std::string const& key);
        bool isCompleted(std::string const& key) const;
        void close(std::string const& key);
        std::string selectedDirectory(std::string const& key) const;

      private:
        Dialog* findDialog(std::string const& key) const;
        Dialog* createDialog(std::string const& key);

      private:
        std::map<std::string, std::unique_ptr<Dialog>> m_dialogs;
    };
} // namespace ImGui::extend