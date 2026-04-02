#pragma once
#include "imgui.h"

class QOpenGLWidget;
class QOpenGLWindow;
bool ImGui_ImplQt_Init(QOpenGLWidget* window);
bool ImGui_ImplQt_Init(QOpenGLWindow* window);
void ImGui_ImplQt_Shutdown();
void ImGui_ImplQt_NewFrame();
