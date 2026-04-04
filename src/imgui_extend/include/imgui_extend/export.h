#pragma once

#if defined(_WIN32)
#    if defined(IMGUI_EXTEND_EXPORTS)
#        define IMGUI_EXTEND_API __declspec(dllexport)
#    else
#        define IMGUI_EXTEND_API __declspec(dllimport)
#    endif
#else
#    define IMGUI_EXTEND_API
#endif