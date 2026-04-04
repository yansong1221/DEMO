/*
The MIT License (MIT)

Copyright (c) 2016 Andre Leiradella

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

#include "imgui_extend/imguial_msgbox.h"
#include "imgui.h"

ImGuiAl::MsgBox::~MsgBox() {}

void
ImGuiAl::MsgBox::Init(std::string const& title,
                      std::string const& icon,
                      std::string const& text,
                      std::vector<std::string> const& captions,
                      bool show_checkbox,
                      CallBackFunc handler)
{
    m_Title = title;
    m_Icon = icon;
    m_Text = text;
    m_Captions = captions;
    m_ShowCheckbox = show_checkbox;
    m_callBack = std::move(handler);
}

int
ImGuiAl::MsgBox::Draw()
{
    if (m_Open)
    {
        m_Open = false;
        ImGui::OpenPopup(m_Title.c_str());
    }

    int index = 0;

    if (ImGui::BeginPopupModal(m_Title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (m_DontAskAgain && m_Selected != 0)
        {
            ImGui::CloseCurrentPopup();
            index = m_Selected;
        }
        else
        {
            if (!m_Icon.empty())
            {
                ImVec2 size = ImGui::CalcTextSize(m_Icon.c_str());
                ImVec2 pos = ImGui::GetCursorPos();
                float save_y = pos.y;
                pos.y += size.y;

                ImGui::SetCursorPos(pos);
                ImGui::Text(m_Icon.c_str());

                pos.x += size.x + pos.x;
                pos.y = save_y;

                ImGui::SetCursorPos(pos);
                ImGui::TextWrapped(m_Text.c_str());
            }
            else
            {
                ImGui::TextWrapped(m_Text.c_str());
            }

            ImGui::Separator();

            if (m_ShowCheckbox)
            {
                ImGui::Checkbox("Don't ask me again", &m_DontAskAgain);
            }

            ImVec2 size = ImVec2(50.0f, 0.0f);
            int count;

            for (count = 0; count < m_Captions.size(); count++)
            {
                if (ImGui::Button(m_Captions[count].c_str(), size))
                {
                    index = count + 1;
                    ImGui::CloseCurrentPopup();

                    if (m_callBack)
                    {
                        m_callBack(index);
                    }
                    break;
                }

                ImGui::SameLine();
            }

            size = ImVec2((4 - count) * 50.0f, 1.0f);
            ImGui::Dummy(size);

            if (m_DontAskAgain)
            {
                m_Selected = index;
            }
        }

        ImGui::EndPopup();
    }

    return index;
}

void
ImGuiAl::MsgBox::Open()
{
    m_Open = true;
}
