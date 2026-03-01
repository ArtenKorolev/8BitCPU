cmake_minimum_required(VERSION 3.10)
project(emu8)

file(GLOB C_SRC 
    "source/*.c"
    "source/machine/apple2/*.c"
    "source/core/*.c"
    "source/utils/*.c"
)

add_compile_options(-Wall -Wextra -Wpedantic -O0)

add_executable(${PROJECT_NAME} ${C_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC 
    source/core 
    source/utils
    source/machine/apple2
)
