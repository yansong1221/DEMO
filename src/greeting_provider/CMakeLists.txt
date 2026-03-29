find_package(CppMicroServices REQUIRED)

set(MODULE greeting_provider)

set(_srcs src/GreetingActivator.cpp)


  usFunctionGetResourceSource(TARGET ${MODULE} OUT _srcs)
  usFunctionGenerateBundleInit(TARGET ${MODULE} OUT _srcs)

  add_library(${MODULE} SHARED ${_srcs})
  set_property(TARGET ${MODULE} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=greeting_provider)
  set_property(TARGET ${MODULE} PROPERTY DEBUG_POSTFIX "")

  target_link_libraries(${MODULE} ${CppMicroServices_LIBRARIES} service_interface imgui)
  target_include_directories(${MODULE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  set_target_properties(${MODULE} PROPERTIES
    LABELS ${MODULE}
    OUTPUT_NAME ${MODULE}
  )

  usFunctionAddResources(TARGET ${MODULE} BUNDLE_NAME ${MODULE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} FILES manifest.json)
  usFunctionEmbedResources(TARGET ${MODULE})
