/*
The MIT License (MIT)

Copyright (c) 2019 Andre Leiradella

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "imgui.h"
#include "textselect.hpp"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <functional>
#include <map>
#include <string>

namespace ImGuiAl
{

    class Crt
    {
      public:
        struct CGA
        {
            enum : ImU32
            {
                Black = IM_COL32(0x00, 0x00, 0x00, 0xff),
                Blue = IM_COL32(0x00, 0x00, 0xaa, 0xff),
                Green = IM_COL32(0x00, 0xaa, 0x00, 0xff),
                Cyan = IM_COL32(0x00, 0xaa, 0xaa, 0xff),
                Red = IM_COL32(0xaa, 0x00, 0x00, 0xff),
                Magenta = IM_COL32(0xaa, 0x00, 0xaa, 0xff),
                Brown = IM_COL32(0xaa, 0x55, 0x00, 0xff),
                White = IM_COL32(0xaa, 0xaa, 0xaa, 0xff),
                Gray = IM_COL32(0x55, 0x55, 0x55, 0xff),
                BrightBlue = IM_COL32(0x55, 0x55, 0xff, 0xff),
                BrightGreen = IM_COL32(0x55, 0xff, 0x55, 0xff),
                BrightCyan = IM_COL32(0x55, 0xff, 0xff, 0xff),
                BrightRed = IM_COL32(0xff, 0x55, 0x55, 0xff),
                BrightMagenta = IM_COL32(0xff, 0x55, 0xff, 0xff),
                Yellow = IM_COL32(0xff, 0xff, 0x55, 0xff),
                BrightWhite = IM_COL32(0xff, 0xff, 0xff, 0xff)
            };
        };

        struct Info
        {
            ImU32 foregroundColor;
            std::string content;
            unsigned metaData;
        };

        Crt(size_t const max_size);

        void setForegroundColor(ImU32 const color);
        void setMetaData(unsigned const meta_data);

        void print(std::string const& text);

        void printf(char const* const format, ...);
        void vprintf(char const* const format, va_list args);
        void scrollToBottom();
        void clear();
        void draw(ImVec2 const& size = ImVec2(0.0f, 0.0f));

      protected:
        virtual bool
        filterItem(Info const& header) const
        {
            return false;
        }

      private:
        void doDrawItems();
        void doScrollToBottom();

        std::vector<Info> _items;
        std::vector<std::size_t> _drawItemIndexes;

        size_t _maxItemCount;

        ImU32 _foregroundColor;
        unsigned _metaData;
        bool _scrollToBottom;

        TextSelect _textSelect;
    };

    class Log : protected Crt
    {
      public:
        typedef Crt::Info Info;

        enum class Level : unsigned int
        {
            Trace = 0x1,
            Debug = 0x2,
            Info = 0x4,
            Warning = 0x8,
            Error = 0x10,
            Critical = 0x20,
            MaskAll = 0x3f,
        };

        Log(size_t const max_size);

        void trace(char const* const format, ...);
        void debug(char const* const format, ...);
        void info(char const* const format, ...);
        void warning(char const* const format, ...);
        void error(char const* const format, ...);
        void critical(char const* const format, ...);

        void trace(char const* const format, va_list args);
        void debug(char const* const format, va_list args);
        void info(char const* const format, va_list args);
        void warning(char const* const format, va_list args);
        void error(char const* const format, va_list args);
        void critical(char const* const format, va_list args);

        void intput(Level level, std::string const& text);

        void
        clear()
        {
            Crt::clear();
        }

        void
        scrollToBottom()
        {
            Crt::scrollToBottom();
        }
        void
        setLevel(unsigned int const level)
        {
            _level = level;
        }
        unsigned int
        getLevel() const
        {
            return _level;
        }

        int draw(ImVec2 const& size = ImVec2(0.0f, 0.0f));

        void setColor(Level const level, ImU32 const color);
        void setLabel(Level const level, char const* const label);

        void setActions(char const* actions[]);

      protected:
        bool filterItem(Info const& header) const override;

      protected:
        struct LogLevelData
        {
            ImU32 textColor = 0;
            ImU32 buttonColor = 0;
            ImU32 buttonHoveredColor = 0;
            std::string label;

            LogLevelData() = default;
            LogLevelData(std::string const& _lable, ImU32 const color) : label(_lable) { setColor(color); }

            inline static ImU32
            changeValue(ImU32 const color, float const delta_value)
            {
                ImVec4 rgba = ImGui::ColorConvertU32ToFloat4(color);

                float h, s, v;
                ImGui::ColorConvertRGBtoHSV(rgba.x, rgba.y, rgba.z, h, s, v);
                v += delta_value;

                if (v < 0.0f)
                {
                    v = 0.0f;
                }
                else if (v > 1.0f)
                {
                    v = 1.0f;
                }

                ImGui::ColorConvertHSVtoRGB(h, s, v, rgba.x, rgba.y, rgba.z);
                return ImGui::ColorConvertFloat4ToU32(rgba);
            }
            inline void
            setColor(ImU32 const color)
            {
                buttonColor = changeValue(color, -0.2f);
                buttonHoveredColor = changeValue(color, -0.1f);
                textColor = color;
            }
        };
        std::map<Level, LogLevelData> _levelData;

        char const* const* _actions;

        unsigned int _level;
        ImGuiTextFilter _filter;
    };

    class Terminal : protected Crt
    {
      public:
        typedef Crt::Info Info;

        Terminal(size_t const max_size,
                 void* const cmd_buf,
                 size_t const cmd_size,
                 std::function<void(Terminal& self, char* const command)>&& execute,
                 std::function<void(Terminal& self, ImGuiInputTextCallbackData* data)>&& callback);

        void
        setForegroundColor(ImU32 const color)
        {
            Crt::setForegroundColor(color);
        }

        void printf(char const* const format, ...);
        void
        vprintf(char const* const format, va_list args)
        {
            Crt::vprintf(format, args);
        }

        void
        clear()
        {
            Crt::clear();
        }
        void
        scrollToBottom()
        {
            Crt::scrollToBottom();
        }

        void draw(ImVec2 const& size = ImVec2(0.0f, 0.0f));

      protected:
        char* _commandBuffer;
        size_t _cmdBufferSize;
        std::function<void(Terminal& self, char* const command)> _execute;
        std::function<void(Terminal& self, ImGuiInputTextCallbackData* data)> _callback;
    };

    template <size_t S, size_t R>
    class BufferedTerminal : public Terminal
    {
      public:
        BufferedTerminal(std::function<void(Terminal& self, char* const command)>&& execute,
                         std::function<void(Terminal& self, ImGuiInputTextCallbackData* data)>&& callback)
            : Terminal(_buffer, S, _commandBuffer, R, std::move(execute), std::move(callback))
        {
        }

      protected:
        uint8_t _buffer[S];
        uint8_t _commandBuffer[R];
    };
} // namespace ImGuiAl
