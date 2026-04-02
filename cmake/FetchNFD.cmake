# FetchHttpLib.cmake
# 用于获取和配置 httplib 库的 CMake 模块

include(FetchContent)

# 保存当前 FOLDER 设置
set(_OLD_USE_FOLDERS ${USE_FOLDERS})
set(CMAKE_FOLDER "3rdparty")

FetchContent_Declare(NFD
  GIT_REPOSITORY    "https://github.com/btzy/nativefiledialog-extended.git" 
  GIT_TAG           "master"
)
FetchContent_MakeAvailable(NFD)

# 恢复 FOLDER 设置
set(CMAKE_FOLDER "")
