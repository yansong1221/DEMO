#include "LogWidget.h"

#include <QCheckBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QVBoxLayout>

LogWidget::LogWidget(QWidget* parent) : ImguiWidget(parent), _terminal(10000) {}

LogWidget::~LogWidget() = default;

void
LogWidget::addLog(spdlog::level::level_enum level, QString const& message)
{
    switch (level)
    {
        case spdlog::level::trace:
            _terminal.intput(ImGuiAl::Log::Level::Trace, message.toStdString());
            break;
        case spdlog::level::debug:
            _terminal.intput(ImGuiAl::Log::Level::Debug, message.toStdString());
            break;
        case spdlog::level::info:
            _terminal.intput(ImGuiAl::Log::Level::Info, message.toStdString());
            break;
        case spdlog::level::warn:
            _terminal.intput(ImGuiAl::Log::Level::Warning, message.toStdString());
            break;
        case spdlog::level::err:
            _terminal.intput(ImGuiAl::Log::Level::Error, message.toStdString());
            break;
        case spdlog::level::critical:
            _terminal.intput(ImGuiAl::Log::Level::Critical, message.toStdString());
            break;
        default:
            break;
    }
}

void
LogWidget::drawImgui()
{
    _terminal.draw();
}

void
LogWidget::initializeGL()
{
    ImguiWidget::initializeGL();
    ImGui::StyleColorsDark();
}
