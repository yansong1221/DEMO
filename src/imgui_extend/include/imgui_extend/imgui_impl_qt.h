#pragma once
#include "export.h"
#include "imgui.h"

class QOpenGLWidget;
class QOpenGLWindow;
IMGUI_EXTEND_API bool     ImGui_ImplQt_Init(QOpenGLWidget* window);
IMGUI_EXTEND_API bool     ImGui_ImplQt_Init(QOpenGLWindow* window);
IMGUI_EXTEND_API void     ImGui_ImplQt_Shutdown();
IMGUI_EXTEND_API void     ImGui_ImplQt_NewFrame();
