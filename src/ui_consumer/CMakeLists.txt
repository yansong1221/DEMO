find_package(CppMicroServices REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(spdlog CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(MODULE ui_consumer)

set(_srcs src/ConsumerActivator.cpp)


  usFunctionGetResourceSource(TARGET ${MODULE} OUT _srcs)
  usFunctionGenerateBundleInit(TARGET ${MODULE} OUT _srcs)

  add_library(${MODULE} SHARED ${_srcs})
  set_property(TARGET ${MODULE} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=${MODULE})
  set_property(TARGET ${MODULE} PROPERTY DEBUG_POSTFIX "")

  target_link_libraries(${MODULE} PRIVATE ${CppMicroServices_LIBRARIES} service_interface Qt5::Widgets spdlog::spdlog) 
  set_target_properties(${MODULE} PROPERTIES
    LABELS ${MODULE}
    OUTPUT_NAME ${MODULE}
  )

  usFunctionAddResources(TARGET ${MODULE} BUNDLE_NAME ${MODULE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} FILES manifest.json)
  usFunctionEmbedResources(TARGET ${MODULE})


