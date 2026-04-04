
set(MODULE ai_agent)

find_package(Boost REQUIRED COMPONENTS asio json beast CONFIG)
find_package(OpenCV REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core)

include(FetchHttpLib)


file(GLOB_RECURSE MODULE_SOURCES 
${CMAKE_CURRENT_SOURCE_DIR}/*.h 
${CMAKE_CURRENT_SOURCE_DIR}/*.hpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.c 
${CMAKE_CURRENT_SOURCE_DIR}/*.ui 
${CMAKE_CURRENT_SOURCE_DIR}/*.json 
${CMAKE_CURRENT_SOURCE_DIR}/*.rc)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_SOURCES})

us_add_bundle(
    TARGET ${MODULE}
    SOURCES ${MODULE_SOURCES}
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    LINK_LIBRARIES imgui imgui_extend SQLiteCpp Boost::asio Boost::json Boost::beast httplib ${OpenCV_LIBS} Qt5::Core
)

set_target_properties(${MODULE} PROPERTIES AUTOMOC ON)