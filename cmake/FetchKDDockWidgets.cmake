# FetchKDDockWidgets.cmake
# 用于获取和配置 KDDockWidgets 库的 CMake 模块

include(FetchContent)

# 保存当前 FOLDER 设置
set(_OLD_USE_FOLDERS ${USE_FOLDERS})
set(CMAKE_FOLDER "3rdparty")

option(KDDockWidgets_QT6 "Build with Qt6" OFF)
# 声明 KDDockWidgets 依赖
FetchContent_Declare(
    KDDockWidgets
    GIT_REPOSITORY https://github.com/KDAB/KDDockWidgets.git
    GIT_TAG v2.4.0
)

# 获取并构建
FetchContent_MakeAvailable(KDDockWidgets)

# 恢复 FOLDER 设置
set(CMAKE_FOLDER "")

# 强制设置 KDDockWidgets 主目标的 C++ 标准
if(TARGET kddockwidgets)
    set_target_properties(kddockwidgets PROPERTIES 
        CXX_STANDARD 23 
        CXX_STANDARD_REQUIRED ON
    )
endif()
