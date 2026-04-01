
SET(MODULE launcher)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(spdlog CONFIG REQUIRED)

include(FetchKDDockWidgets)

file(GLOB_RECURSE MODULE_SOURCES 
${CMAKE_CURRENT_SOURCE_DIR}/*.h 
${CMAKE_CURRENT_SOURCE_DIR}/*.hpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.c 
${CMAKE_CURRENT_SOURCE_DIR}/*.ui 
${CMAKE_CURRENT_SOURCE_DIR}/*.rc)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_SOURCES})


add_executable(${MODULE} ${MODULE_SOURCES})

# 添加当前目录为包含目录，使子目录能找到根目录的头文件
target_include_directories(${MODULE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(${MODULE} PRIVATE Qt5::Widgets KDAB::kddockwidgets CppMicroServices usLogService spdlog::spdlog imgui)
target_link_libraries(${MODULE} PRIVATE service_interface)
set_target_properties(${MODULE} PROPERTIES AUTOMOC ON)

# 定义 Bundle 名称（用于 CppMicroServices 的 BundleActivator）
target_compile_definitions(${MODULE} PRIVATE US_BUNDLE_NAME=${MODULE})

if(MSVC)
    target_compile_options(${MODULE} PRIVATE /utf-8)
endif()
