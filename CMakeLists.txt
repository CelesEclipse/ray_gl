cmake_minimum_required(VERSION 3.10)
project(raylibshit C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(RAYLIB_SRC_DIR "C:/raylibx/raylib/src")
include_directories(${RAYLIB_SRC_DIR})

set(SOURCES 
    src/main.c 
    src/core/core.h
    src/core/core.c
    src/utils/utils.h
    src/utils/utils.c
    src/core/camera/camera.h
    src/core/camera/camera.c
    src/core/enemies/enemy.h
    src/core/enemies/enemy.c
    src/core/player/player.c 
    src/core/player/player.h
    src/core/hud/ui.h
    src/core/hud/ui.c
    src/core/collision/collision.h
    src/core/collision/collision.c
    src/core/models/animations/anim_controller.h
    src/core/models/animations/anim_controller.c
    src/physics/geometry.h
    src/physics/geometry.c
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
