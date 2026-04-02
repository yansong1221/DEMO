#pragma once

#include "imgui.h"
#include "export.h"

IMGUI_EXTEND_API bool ImGui_ImplQtOpenGL3_Init(const char* glsl_version = nullptr);
IMGUI_EXTEND_API void ImGui_ImplQtOpenGL3_Shutdown();
IMGUI_EXTEND_API void ImGui_ImplQtOpenGL3_NewFrame();
IMGUI_EXTEND_API void ImGui_ImplQtOpenGL3_RenderDrawData(ImDrawData* draw_data);

IMGUI_EXTEND_API bool ImGui_ImplQtOpenGL3_CreateFontsTexture();
IMGUI_EXTEND_API void ImGui_ImplQtOpenGL3_DestoryFontsTexture();
IMGUI_EXTEND_API bool ImGui_ImplQtOpenGL3_CreateDeviceObjects();
IMGUI_EXTEND_API void ImGui_ImplQtOpenGL3_DestoryDeviceObjects();
