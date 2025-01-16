cmake_minimum_required(VERSION 3.11)
project(TP3 C)
FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)

set(CMAKE_C_STANDARD 99)

bison_target(parser parse.y ${CMAKE_CURRENT_BINARY_DIR}/parse.tab.c)
flex_target(tokens tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.c)
ADD_FLEX_BISON_DEPENDENCY(tokens parser)

message(STATUS "successfully ran bison and flex!")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(TP3 ${BISON_parser_OUTPUTS} ${FLEX_tokens_OUTPUTS} common.c common.h conf.h pm.c pm.h pt.c pt.h tlb.c tlb.h tokens.c vmm.c vmm.h)