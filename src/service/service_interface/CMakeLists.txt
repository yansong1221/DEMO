

find_package(yaml-cpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(service_interface INTERFACE)
target_include_directories(service_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(service_interface INTERFACE yaml-cpp::yaml-cpp spdlog::spdlog)