# FetchImGui.cmake
# 用于获取和配置 Dear ImGui 库的 CMake 模块

if(NOT TARGET imgui)
    include(FetchContent)

    find_package(Freetype REQUIRED)

        # 声明 ImGui 依赖
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.6-docking
    )
    FetchContent_MakeAvailable(imgui)

    # 配置 ImGui 源码文件
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
    file(GLOB IMGUI_SOURCES
        "${IMGUI_DIR}/*.cpp"
        "${IMGUI_DIR}/misc/freetype/*.cpp"
        "${IMGUI_DIR}/misc/cpp/*.cpp"
    )
    
    # 创建 ImGui 静态库
    add_library(imgui STATIC ${IMGUI_SOURCES})
    
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/misc/cpp
    )
    
    target_link_libraries(imgui PRIVATE Freetype::Freetype)

    # 设置 VS 项目文件夹
    set_target_properties(imgui PROPERTIES FOLDER "3rdparty")

endif()
