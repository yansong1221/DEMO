find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(spdlog CONFIG REQUIRED)

FetchContent_Declare(
    KDDockWidgets
    GIT_REPOSITORY https://github.com/KDAB/KDDockWidgets.git
    GIT_TAG v2.4.0
)
FetchContent_MakeAvailable(KDDockWidgets)

# 强制设置KDDockWidgets目标的C++标准
set_target_properties(kddockwidgets PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

add_executable(host_app
    main.cpp
    MainWindow.cpp
    MainWindow.h
    BundleManagerDockWidget.cpp
    BundleManagerDockWidget.h
    TaskServiceDockWidget.cpp
    TaskServiceDockWidget.h
    PluginBundleActionDelegate.cpp
    PluginBundleActionDelegate.h
    PluginBundleTableModel.cpp
    PluginBundleTableModel.h
    TaskServiceTableModel.cpp
    TaskServiceTableModel.h
    TaskServiceActionDelegate.cpp
    TaskServiceActionDelegate.h
    thread.hpp
    imgui/imgui_impl_qt_opengl3.cpp
    imgui/imgui_impl_qt_opengl3.h
    imgui/imgui_impl_qt.h
    imgui/imgui_impl_qt.cpp
    imgui/QImguiWidget.cpp
    imgui/QImguiWidget.h
    TaskServiceConfigDialog.cpp
    TaskServiceConfigDialog.h
)

target_link_libraries(host_app PRIVATE Qt5::Widgets KDAB::kddockwidgets CppMicroServices spdlog::spdlog imgui)
target_link_libraries(host_app PRIVATE service_interface)
set_target_properties(host_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(MSVC)
    target_compile_options(host_app PRIVATE /utf-8)
endif()
