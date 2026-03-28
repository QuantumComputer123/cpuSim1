cmake_minimum_required(VERSION 3.10)

if(APPLE)
    set(CMAKE_PREFIX_PATH "/opt/homebrew")
endif()

project(MyProject)

find_package(raylib REQUIRED)

add_executable(main 
main.cpp
)
target_compile_options(main PRIVATE -O3)
target_link_libraries(main PRIVATE raylib)

if(APPLE)
    target_link_libraries(main PRIVATE 
        "-framework IOKit" 
        "-framework Cocoa" 
        "-framework OpenGL"
    )
endif()