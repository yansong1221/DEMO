# src/common/CMakeLists.txt
# 公共组件库

find_package(CppMicroServices REQUIRED)

set(COMMON_SRCS
    src/Logger.cpp
)

add_library(common STATIC ${COMMON_SRCS})

target_include_directories(common PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common PUBLIC 
    CppMicroServices
)

# 设置 VS 文件夹归类
set_target_properties(common PROPERTIES FOLDER "common")
