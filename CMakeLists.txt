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

add_executable(${CMAKE_PROJECT_NAME}
    "src/main.c"
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(${CMAKE_PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)