# FetchHttpLib.cmake
# 用于获取和配置 httplib 库的 CMake 模块

include(FetchContent)

# 保存当前 FOLDER 设置
set(_OLD_USE_FOLDERS ${USE_FOLDERS})
set(CMAKE_FOLDER "3rdparty")

# 声明 httplib 依赖
FetchContent_Declare(
    httplib
    GIT_REPOSITORY "https://github.com/yansong1221/httplib.git"
    GIT_TAG master
)

# 获取并构建httplib

FetchContent_MakeAvailable(httplib)

# 恢复 FOLDER 设置
set(CMAKE_FOLDER "")
