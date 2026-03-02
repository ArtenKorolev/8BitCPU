cmake_minimum_required(VERSION 3.10)
project(emu8)

file(GLOB C_SRC 
    "source/*.c"
    "source/machine/apple2/*.c"
    "source/core/*.c"
    "source/utils/*.c"
)

include_directories(
    source/core 
    source/utils
    source/machine
    source/machine/apple2
)

add_compile_options(-Wall -Wextra -Wpedantic -O2)

add_executable(${PROJECT_NAME} ${C_SRC})

