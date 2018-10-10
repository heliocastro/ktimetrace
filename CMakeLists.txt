cmake_minimum_required(VERSION 3.1.0)

project(ktimetrace)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(zlib REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(ktimetrace)