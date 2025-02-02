cmake_minimum_required(VERSION 3.31)
project(ProjektBartek)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
        main.cpp
        common.cpp
        KapitanStatku.cpp
        KapitanPortu.cpp
        Pasazer.cpp
)
add_library(common STATIC common.cpp)

add_executable(projekt main.cpp)
target_link_libraries(projekt PRIVATE common)

add_executable(KapitanStatku KapitanStatku.cpp)
target_link_libraries(KapitanStatku PRIVATE common)

add_executable(KapitanPortu KapitanPortu.cpp)
target_link_libraries(KapitanPortu PRIVATE common)

add_executable(Pasazer Pasazer.cpp)
target_link_libraries(Pasazer PRIVATE common)

