cmake_minimum_required(VERSION 3.16)

project(CppMicroServicesDemo LANGUAGES CXX)

# 添加 cmake 模块路径
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# 检测是否为顶层项目
get_directory_property(_HAS_PARENT PARENT_DIRECTORY)
if(_HAS_PARENT)
    set(IS_ROOT_PROJECT OFF)
else()
    set(IS_ROOT_PROJECT ON)
endif()

# 架构和输出路径
if(IS_ROOT_PROJECT)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(MACHINE_TYPE x64)
    else()
        set(MACHINE_TYPE x86)
    endif()
    foreach(config Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${config} config_upper)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_CURRENT_SOURCE_DIR}/bin/${MACHINE_TYPE}/${config}/_lib)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_CURRENT_SOURCE_DIR}/bin/${MACHINE_TYPE}/${config})
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_CURRENT_SOURCE_DIR}/bin/${MACHINE_TYPE}/${config})
    endforeach()

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. 查找包
find_package(CppMicroServices REQUIRED CONFIG)

# 2. 获取第三方库
include(FetchImGui)


add_subdirectory(src/service_interface)
add_subdirectory(src/greeting_provider)
add_subdirectory(src/greeting_consumer)
add_subdirectory(src/ui_consumer)
add_subdirectory(src/host)
