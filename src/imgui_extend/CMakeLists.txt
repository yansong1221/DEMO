
set(MODULE imgui_extend)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)


file(GLOB_RECURSE MODULE_SOURCES 
${CMAKE_CURRENT_SOURCE_DIR}/*.h 
${CMAKE_CURRENT_SOURCE_DIR}/*.hpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.c 
${CMAKE_CURRENT_SOURCE_DIR}/*.ui 
${CMAKE_CURRENT_SOURCE_DIR}/*.json 
${CMAKE_CURRENT_SOURCE_DIR}/*.rc)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_SOURCES})

add_library(${MODULE} SHARED ${MODULE_SOURCES})

target_include_directories(${MODULE} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${MODULE} PUBLIC 
    imgui
    PRIVATE Qt5::Core Qt5::Widgets
    PUBLIC service_interface
)
target_compile_definitions(${MODULE} PRIVATE IMGUI_EXTEND_EXPORTS)
target_compile_definitions(${MODULE} PUBLIC NOMINMAX)
set_target_properties(${MODULE} PROPERTIES AUTOMOC ON)

if(MSVC)
    target_compile_options(${MODULE} PRIVATE /utf-8)
endif()