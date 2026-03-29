# FetchKDDockWidgets.cmake
# 用于获取和配置 KDDockWidgets 库的 CMake 模块

include(FetchContent)

# 声明 KDDockWidgets 依赖
FetchContent_Declare(
    KDDockWidgets
    GIT_REPOSITORY https://github.com/KDAB/KDDockWidgets.git
    GIT_TAG v2.4.0
)

# 获取并构建
FetchContent_MakeAvailable(KDDockWidgets)

# 强制设置 KDDockWidgets 目标的 C++ 标准
set_target_properties(kddockwidgets PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
