cmake_minimum_required(VERSION 3.10)
project(raylibshit C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(RAYLIB_SRC_DIR "C:/raylib/raylib/src")
include_directories(${RAYLIB_SRC_DIR})

set(SOURCES 
    src/main.c 
    src/core/enemies/enemy.h
    src/core/enemies/enemy.c
    src/core/player/player.c 
    src/core/player/player.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} 
    "${RAYLIB_SRC_DIR}/libraylib.a"
    opengl32
    gdi32
    winmm
)

if(EXISTS "${CMAKE_BINARY_DIR}")
    message(STATUS "Clearing old build directory: ${CMAKE_BINARY_DIR}")
    file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}")
endif()
