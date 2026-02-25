cmake_minimum_required(VERSION 3.10)
project(emu8)

file(GLOB C_SRC "source/*.c")

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(${PROJECT_NAME} ${C_SRC})
