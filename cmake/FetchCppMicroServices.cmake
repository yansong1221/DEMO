# FetchKMicroServices.cmake
# 用于获取和配置 Cpp 库的 CMake 模块

include(FetchContent)

# 保存当前 FOLDER 设置
set(_OLD_USE_FOLDERS ${USE_FOLDERS})
set(CMAKE_FOLDER "3rdparty")

# 声明 CppMicroServices 依赖
FetchContent_Declare(
    CppMicroServices
    GIT_REPOSITORY https://github.com/yansong1221/CppMicroServices.git
    GIT_TAG development
)

# 获取并构建CppMicroServices

FetchContent_MakeAvailable(CppMicroServices)

# 恢复 FOLDER 设置
set(CMAKE_FOLDER "")

# 强制设置 CppMicroServices 主目标的 C++ 标准
if(TARGET cppmicroservices)
    set_target_properties(cppmicroservices PROPERTIES 
        CXX_STANDARD 23 
        CXX_STANDARD_REQUIRED ON
    )
endif()
