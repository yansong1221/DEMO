# BundleUtils.cmake
# 提供 Bundle 相关的通用 CMake 函数

# 设置 Bundle 目标的输出目录到 bin/<arch>/<config>/bundles/
function(us_setup_bundle_output target_name)
    # 为每种配置设置输出目录
    foreach(config Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER ${config} config_upper)
        set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}}/bundles
            LIBRARY_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper}}/bundles
            ARCHIVE_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper}}/bundles
        )
    endforeach()
endfunction()

# 完整的 Bundle 构建函数
# us_add_bundle(
#     TARGET <target_name>           # 目标名称（必需）
#     SOURCES <source_files...>      # 源文件列表（必需）
#     [INCLUDE_DIRS <dirs...>]       # 额外的包含目录（可选）
#     [LINK_LIBRARIES <libs...>]     # 额外的链接库（可选）
#     [BUNDLE_NAME <name>]           # Bundle 名称，默认与 TARGET 相同（可选）
# )
function(us_add_bundle)
    cmake_parse_arguments(ARG "" "TARGET;BUNDLE_NAME" "SOURCES;INCLUDE_DIRS;LINK_LIBRARIES" ${ARGN})
    
    # 验证必需参数
    if(NOT ARG_TARGET)
        message(FATAL_ERROR "us_add_bundle: TARGET is required")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "us_add_bundle: SOURCES is required")
    endif()
    
    # 设置默认 Bundle 名称
    if(NOT ARG_BUNDLE_NAME)
        set(ARG_BUNDLE_NAME ${ARG_TARGET})
    endif()
    
    # 查找 CppMicroServices
    find_package(CppMicroServices REQUIRED)
    
    # 生成 Bundle 资源源文件
    usFunctionGetResourceSource(TARGET ${ARG_TARGET} OUT _resource_srcs)
    usFunctionGenerateBundleInit(TARGET ${ARG_TARGET} OUT _bundle_init_srcs)
    
    # 创建共享库
    add_library(${ARG_TARGET} SHARED ${ARG_SOURCES} ${_resource_srcs} ${_bundle_init_srcs})
    
    # 设置 Bundle 属性
    set_property(TARGET ${ARG_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=${ARG_BUNDLE_NAME})
    set_property(TARGET ${ARG_TARGET} PROPERTY DEBUG_POSTFIX "")
    
    # 设置基本属性
    set_target_properties(${ARG_TARGET} PROPERTIES
        LABELS ${ARG_TARGET}
        OUTPUT_NAME ${ARG_TARGET}
        FOLDER "bundles"
    )
    
    # 设置 bundles 输出目录
    us_setup_bundle_output(${ARG_TARGET})
    
    # 链接基础库
    target_link_libraries(${ARG_TARGET} 
        ${CppMicroServices_LIBRARIES} 
        service_interface
        ${ARG_LINK_LIBRARIES}
    )
    
    # 添加包含目录
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${ARG_TARGET} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()
    
    # 添加资源文件（如果存在 manifest.json）
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json)
        usFunctionAddResources(
            TARGET ${ARG_TARGET} 
            BUNDLE_NAME ${ARG_BUNDLE_NAME} 
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
            FILES manifest.json
        )
        usFunctionEmbedResources(TARGET ${ARG_TARGET})
    endif()
endfunction()
