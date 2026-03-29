find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(spdlog CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(host_app
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    PluginBundleActionDelegate.cpp
    PluginBundleActionDelegate.h
    PluginBundleTableModel.cpp
    PluginBundleTableModel.h
    TaskServiceTableModel.cpp
    TaskServiceTableModel.h
    TaskServiceActionDelegate.cpp
    TaskServiceActionDelegate.h
    thread.hpp
)

target_link_libraries(host_app PRIVATE Qt5::Widgets CppMicroServices spdlog::spdlog)
target_link_libraries(host_app PRIVATE service_interface)
set_target_properties(host_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(MSVC)
    target_compile_options(host_app PRIVATE /utf-8)
endif()
