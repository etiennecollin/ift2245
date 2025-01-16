cmake_minimum_required(VERSION 3.11)
project(TP0 C)

set(CMAKE_C_STANDARD 11)

option(ENABLE_ADDRESS_SANITIZER "Enable address sanitizer" OFF)
option(ENABLE_LEAK_SANITIZER "Enable address sanitizer" OFF)
option(ENABLE_UNDEFINED_SANITIZER "Enable undefined behavior sanitizer" OFF)
option(ENABLE_MEMORY_SANITIZER "Enable memory sanitizer" OFF)

if (ENABLE_ADDRESS_SANITIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
endif()

if (ENABLE_LEAK_SANITIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak")
endif()

if (ENABLE_UNDEFINED_SANITIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
endif()

if (ENABLE_MEMORY_SANITIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=memory")
endif()

add_executable(TP0 main.c main.h)
