us_add_bundle(
    TARGET greeting_provider
    SOURCES src/GreetingActivator.cpp
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    LINK_LIBRARIES imgui
)
