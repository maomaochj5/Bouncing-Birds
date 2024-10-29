cmake_minimum_required(VERSION 3.28)
project(Games_1)

set(CMAKE_CXX_STANDARD 23)

add_executable(Games_1 src/Test_2.cpp)

# 添加下面SFML配置
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR ${CMAKE_SOURCE_DIR}/Dependencies/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window graphics audio network REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/Dependencies/SFML/include)
target_link_libraries(Games_1 sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
# 复制资源到生成的exe文件夹

file(COPY Images DESTINATION ${CMAKE_BINARY_DIR})