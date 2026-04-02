#pragma once

#include "imgui.h"

bool ImGui_ImplQtOpenGL3_Init(char const* glsl_version = nullptr);
void ImGui_ImplQtOpenGL3_Shutdown();
void ImGui_ImplQtOpenGL3_NewFrame();
void ImGui_ImplQtOpenGL3_RenderDrawData(ImDrawData* draw_data);

bool ImGui_ImplQtOpenGL3_CreateFontsTexture();
void ImGui_ImplQtOpenGL3_DestoryFontsTexture();
bool ImGui_ImplQtOpenGL3_CreateDeviceObjects();
void ImGui_ImplQtOpenGL3_DestoryDeviceObjects();
