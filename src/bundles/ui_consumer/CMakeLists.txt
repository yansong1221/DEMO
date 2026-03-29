find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(spdlog CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

us_add_bundle(
    TARGET ui_consumer
    SOURCES src/ConsumerActivator.cpp
    LINK_LIBRARIES Qt5::Widgets spdlog::spdlog
)
