cmake_minimum_required(VERSION 3.28)
project(Games_1)

set(CMAKE_CXX_STANDARD 23)

# Windows 特定配置
if(WIN32)
    enable_language(RC)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/src/resources.rc")
endif()

# 添加可执行文件
add_executable(Games_1 WIN32
        src/Main.cpp
        src/Game.h
        src/Menu.h
    ${APP_ICON_RESOURCE_WINDOWS}
)

# SFML配置
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR ${CMAKE_SOURCE_DIR}/Dependencies/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window graphics audio network REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/Dependencies/SFML/include)
target_link_libraries(Games_1 sfml-system sfml-window sfml-graphics sfml-audio sfml-network)

# 复制资源到生成的exe文件夹
file(COPY Images DESTINATION ${CMAKE_BINARY_DIR})

set(CMAKE_EXE_LINKER_FLAGS -static)