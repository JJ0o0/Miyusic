cmake_minimum_required(VERSION 3.20)

project(
    miyusic
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(cJSON CONFIG REQUIRED)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_executable(${CMAKE_PROJECT_NAME}
    "src/main.c"
    "src/player.c"
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CURSES_INCLUDE_DIRS}
        "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE 
        cjson
        ${CURSES_LIBRARIES}
)

target_compile_options(${CMAKE_PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_compile_definitions(${CMAKE_PROJECT_NAME} 
    PRIVATE
        _POSIX_C_SOURCE=200809L
)