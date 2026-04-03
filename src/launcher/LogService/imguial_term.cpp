#include "imguial_term.h"

#include "imgui_extend/component.h"
#include <algorithm>
#include <ctype.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QCoreApplication>

ImGuiAl::Crt::Crt(size_t const max_size)
    : _maxItemCount(max_size)
    , _foregroundColor(CGA::White)
    , _metaData(0)
    , _scrollToBottom(false)
    , _textSelect(
          [this](size_t idx) -> std::string_view
          {
              using namespace std::string_view_literals;
              if (idx >= _drawItemIndexes.size())
              {
                  return "NULL"sv;
              }
              auto index = _drawItemIndexes[idx];
              return _items[index].content;
          },
          [this]() { return _drawItemIndexes.size(); })
{
    _items.reserve(_maxItemCount);
}

void
ImGuiAl::Crt::setForegroundColor(ImU32 const color)
{
    _foregroundColor = color;
}

void
ImGuiAl::Crt::setMetaData(unsigned const meta_data)
{
    _metaData = meta_data;
}

void
ImGuiAl::Crt::print(std::string const& text)
{
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line))
    {
        if (line.empty())
        {
            continue;
        }
        Info header;
        header.foregroundColor = _foregroundColor;
        header.content.swap(line);
        header.metaData = _metaData;
        _items.push_back(std::move(header));
    }
}

void
ImGuiAl::Crt::printf(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void
ImGuiAl::Crt::vprintf(char const* const format, va_list args)
{
    std::string buffer;
    int printed_chars = vsnprintf(buffer.data(), buffer.size(), format, args);
    if (printed_chars >= buffer.size())
    {
        buffer.resize(printed_chars);
        vsnprintf(buffer.data(), buffer.size(), format, args);
    }
    print(buffer);
}

void
ImGuiAl::Crt::scrollToBottom()
{
    _scrollToBottom = true;
}

void
ImGuiAl::Crt::clear()
{
    _items.clear();
    _drawItemIndexes.clear();
    _textSelect.clearSelection();
}

void
ImGuiAl::Crt::draw(ImVec2 const& size)
{
    char id[64];
    snprintf(id, sizeof(id), "ImGuiAl::Crt@%p", static_cast<void*>(this));

    ImGui::BeginChild(id, size, false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 1.0f));

    doDrawItems();
    doScrollToBottom();

    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void
ImGuiAl::Crt::doDrawItems()
{
    if (!_textSelect.hasSelection() && _items.size() > _maxItemCount)
    {
        auto overflowCount = _items.size() - _maxItemCount;
        _items.erase(_items.begin(), _items.begin() + overflowCount);
    }

    std::vector<std::size_t> newDrawItemIndexes;
    for (int i = 0; i < _items.size(); ++i)
    {
        if (filterItem(_items[i]))
        {
            continue;
        }
        newDrawItemIndexes.push_back(i);
    }

    if (_textSelect.hasSelection()
        && !std::includes(newDrawItemIndexes.begin(),
                          newDrawItemIndexes.end(),
                          _drawItemIndexes.begin(),
                          _drawItemIndexes.end()))
    {
        _textSelect.clearSelection();
    }
    _drawItemIndexes.swap(newDrawItemIndexes);

    ImGuiListClipper clipper;
    clipper.Begin(_drawItemIndexes.size());
    while (clipper.Step())
    {
        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
        {
            auto index = _drawItemIndexes[line_no];

            ImGui::PushStyleColor(ImGuiCol_Text, _items[index].foregroundColor);
            ImGui::TextUnformatted(_items[index].content.c_str());
            ImGui::PopStyleColor();
        }
    }
    clipper.End();
    _textSelect.update();
    if (ImGui::BeginPopupContextWindow())
    {
        ImGui::BeginDisabled(!_textSelect.hasSelection());
        if (ImGui::MenuItem(QCoreApplication::translate("ImGuiAl", "Copy").toUtf8(), "Ctrl+C"))
        {
            _textSelect.copy();
        }
        ImGui::EndDisabled();

        if (ImGui::MenuItem(QCoreApplication::translate("ImGuiAl", "Select All").toUtf8(), "Ctrl+A"))
        {
            _textSelect.selectAll();
        }

        if (ImGui::MenuItem(QCoreApplication::translate("ImGuiAl", "Clear").toUtf8()))
        {
            clear();
        }
        ImGui::EndPopup();
    }
}

void
ImGuiAl::Crt::doScrollToBottom()
{
    if (!_scrollToBottom)
    {
        float scrollY = ImGui::GetScrollY();
        float maxScrollY = ImGui::GetScrollMaxY();
        if (scrollY >= maxScrollY)
        {
            _scrollToBottom = true;
        }

        if (_textSelect.hasSelection())
        {
            _scrollToBottom = false;
        }
    }

    if (_scrollToBottom)
    {
        ImGui::SetScrollHereY();
        _scrollToBottom = false;
    }
}

ImGuiAl::Log::Log(size_t const max_size) : Crt(max_size), _actions(nullptr), _level((unsigned int)Level::MaskAll)
{
    _levelData[Level::Trace] = LogLevelData("Trace", CGA::White);
    _levelData[Level::Debug] = LogLevelData("Debug", CGA::BrightBlue);
    _levelData[Level::Info] = LogLevelData("Info", CGA::BrightGreen);
    _levelData[Level::Warning] = LogLevelData("Warning", CGA::Yellow);
    _levelData[Level::Error] = LogLevelData("Error", CGA::BrightRed);
    _levelData[Level::Critical] = LogLevelData("Critical", IM_COL32(0xFF, 0x00, 0x00, 0xff));
}

void
ImGuiAl::Log::debug(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    debug(format, args);
    va_end(args);
}

void
ImGuiAl::Log::info(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    info(format, args);
    va_end(args);
}

void
ImGuiAl::Log::warning(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    warning(format, args);
    va_end(args);
}

void
ImGuiAl::Log::error(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    error(format, args);
    va_end(args);
}

void
ImGuiAl::Log::critical(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    critical(format, args);
    va_end(args);
}

void
ImGuiAl::Log::intput(Level level, std::string const& text)
{
    setForegroundColor(_levelData[level].textColor);
    setMetaData(static_cast<unsigned>(level));
    print(text);
}

void
ImGuiAl::Log::trace(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    trace(format, args);
    va_end(args);
}

void
ImGuiAl::Log::critical(char const* const format, va_list args)
{
    setForegroundColor(_levelData[Level::Critical].textColor);
    setMetaData(static_cast<unsigned>(Level::Critical));
    vprintf(format, args);
}

void
ImGuiAl::Log::trace(char const* const format, va_list args)
{
    setForegroundColor(_levelData[Level::Trace].textColor);
    setMetaData(static_cast<unsigned>(Level::Trace));
    vprintf(format, args);
}

void
ImGuiAl::Log::debug(char const* const format, va_list args)
{
    setForegroundColor(_levelData[Level::Debug].textColor);
    setMetaData(static_cast<unsigned>(Level::Debug));
    vprintf(format, args);
}

void
ImGuiAl::Log::info(char const* const format, va_list args)
{
    setForegroundColor(_levelData[Level::Info].textColor);
    setMetaData(static_cast<unsigned>(Level::Info));
    vprintf(format, args);
}

void
ImGuiAl::Log::warning(char const* const format, va_list args)
{
    setForegroundColor(_levelData[Level::Warning].textColor);
    setMetaData(static_cast<unsigned>(Level::Warning));
    vprintf(format, args);
}

void
ImGuiAl::Log::error(char const* const format, va_list args)
{
    setForegroundColor(_levelData[Level::Error].textColor);
    setMetaData(static_cast<unsigned>(Level::Error));
    vprintf(format, args);
}

int
ImGuiAl::Log::draw(ImVec2 const& size)
{
    int action = 0;

    for (unsigned i = 0; _actions != nullptr && _actions[i] != nullptr; i++)
    {
        if (i != 0)
        {
            ImGui::SameLine();
        }

        if (ImGui::Button(_actions[i]))
        {
            action = i + 1;
        }
    }

    if (ImGui::CollapsingHeader(QCoreApplication::translate("ImGuiAl", "Log display filtering").toUtf8()))
    {
        ImGui::BeginGroup();

        _filter.Draw(QCoreApplication::translate("ImGuiAl", "Filter").toUtf8());
        ImGui::Separator();
        if (ImGui::extend::Button(QCoreApplication::translate("ImGuiAl", "Select All").toUtf8()))
        {
            _level = (unsigned int)Level::MaskAll;
        }
        ImGui::SameLine();
        if (ImGui::extend::Button(QCoreApplication::translate("ImGuiAl", "Deselect all").toUtf8()))
        {
            _level = 0;
        }
        ImGui::SameLine();

        for (auto const& item : _levelData)
        {
            if (ImGui::CheckboxFlags(item.second.label.c_str(), &_level, (unsigned int)item.first))
            {
                scrollToBottom();
            }
            ImGui::SameLine();
        }
        ImGui::EndGroup();
        ImGui::Separator();
    }
    Crt::draw(size);

    return action;
}

void
ImGuiAl::Log::setColor(Level const level, ImU32 const color)
{
    _levelData[level].setColor(color);
}

void
ImGuiAl::Log::setLabel(Level const level, char const* const label)
{
    _levelData[level].label = label;
}

void
ImGuiAl::Log::setActions(char const* actions[])
{
    _actions = actions;
}

bool
ImGuiAl::Log::filterItem(Info const& header) const
{
    unsigned const level = static_cast<unsigned>(_level);

    bool show = (level & header.metaData) == header.metaData;
    show = show && _filter.PassFilter(header.content.c_str());

    return !show;
}

ImGuiAl::Terminal::Terminal(size_t const max_size,
                            void* const cmd_buf,
                            size_t const cmd_size,
                            std::function<void(Terminal& self, char* const command)>&& execute,
                            std::function<void(Terminal& self, ImGuiInputTextCallbackData* data)>&& callback)
    : Crt(max_size)
    , _commandBuffer(static_cast<char*>(cmd_buf))
    , _cmdBufferSize(cmd_size)
    , _execute(std::move(execute))
    , _callback(std::move(callback))
{
    *_commandBuffer = 0;
}

void
ImGuiAl::Terminal::printf(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void
ImGuiAl::Terminal::draw(ImVec2 const& size)
{
    ImVec2 new_size(size.x, size.y - ImGui::GetTextLineHeight() - ImGui::GetTextLineHeightWithSpacing());
    Crt::draw(new_size);
    ImGui::Separator();

    ImGuiInputTextFlags const flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

    static auto const callback = [](ImGuiInputTextCallbackData* data) -> int
    {
        auto const self = static_cast<Terminal*>(data->UserData);
        self->_callback(*self, data);
        return 0;
    };

    bool reclaim_focus = false;

    if (ImGui::InputText("Command", _commandBuffer, _cmdBufferSize, flags, callback, this))
    {
        char* begin = _commandBuffer;

        while (*begin != 0 && isspace(*begin))
        {
            begin++;
        }

        if (*begin != 0)
        {
            char* end = begin + strlen(begin) - 1;

            while (isspace(*end))
            {
                end--;
            }

            end[1] = 0;
            _execute(*this, begin);
        }

        reclaim_focus = true;
    }

    ImGui::SetItemDefaultFocus();

    if (reclaim_focus)
    {
        ImGui::SetKeyboardFocusHere(-1);
    }
}
