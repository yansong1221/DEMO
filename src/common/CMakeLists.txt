# src/common/CMakeLists.txt
# 公共组件库

find_package(CppMicroServices REQUIRED)
find_package(Boost REQUIRED COMPONENTS beast CONFIG)

file(GLOB_RECURSE MODULE_SOURCES 
${CMAKE_CURRENT_SOURCE_DIR}/*.h 
${CMAKE_CURRENT_SOURCE_DIR}/*.hpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.c 
${CMAKE_CURRENT_SOURCE_DIR}/*.ui 
${CMAKE_CURRENT_SOURCE_DIR}/*.json 
${CMAKE_CURRENT_SOURCE_DIR}/*.rc)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_SOURCES})

add_library(common STATIC ${MODULE_SOURCES})

target_include_directories(common PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common PUBLIC 
    CppMicroServices
    usLogService
    Boost::beast
)
