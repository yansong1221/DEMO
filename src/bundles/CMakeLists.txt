# src/bundles/CMakeLists.txt
# 自动处理所有 bundle 子目录

# 获取当前目录下所有子目录（即各个 bundle）
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(child ${children})
    # 检查是否为目录且包含 CMakeLists.txt
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child} AND 
       EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt)
        # 添加子目录
        add_subdirectory(${child})
    endif()
endforeach()
