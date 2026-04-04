#pragma once
#include "export.h"

class QWidget;
class QWindow;

namespace QtImGui
{

    typedef void* RenderRef;

#ifdef QT_WIDGETS_LIB
    IMGUI_EXTEND_API RenderRef initialize(QWidget* window, bool defaultRender = true);
#endif

    IMGUI_EXTEND_API RenderRef initialize(QWindow* window, bool defaultRender = true);
    IMGUI_EXTEND_API void newFrame(RenderRef ref = nullptr);
    IMGUI_EXTEND_API void render(RenderRef ref = nullptr);

} // namespace QtImGui
