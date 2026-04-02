
set(MODULE common)

find_package(CppMicroServices REQUIRED)
find_package(Boost REQUIRED COMPONENTS beast CONFIG)
find_package(OpenCV REQUIRED)

file(GLOB_RECURSE MODULE_SOURCES 
${CMAKE_CURRENT_SOURCE_DIR}/*.h 
${CMAKE_CURRENT_SOURCE_DIR}/*.hpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.c 
${CMAKE_CURRENT_SOURCE_DIR}/*.ui 
${CMAKE_CURRENT_SOURCE_DIR}/*.json 
${CMAKE_CURRENT_SOURCE_DIR}/*.rc)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_SOURCES})

add_library(${MODULE} STATIC ${MODULE_SOURCES})

target_include_directories(${MODULE} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${MODULE} PUBLIC 
    CppMicroServices
    usLogService
    Boost::beast
    ${OpenCV_LIBS}
)

if(MSVC)
    target_compile_options(${MODULE} PRIVATE /utf-8)
endif()