project(emu8)
cmake_minimum_required(VERSION 3.10)

file(GLOB C_SRC "source/*.c")

add_executable(${PROJECT_NAME} ${C_SRC})
