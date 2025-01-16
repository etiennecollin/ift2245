cmake_minimum_required(VERSION 3.11)
project(check_tests C)

set(CMAKE_C_STANDARD 99)

add_executable(check_tests checks.c call_by_string.c call_by_string.h check_utils.h ../src/common.c ../src/common.h ../src/conf.h ../src/pm.c ../src/pm.h ../src/pt.c ../src/pt.h ../src/tlb.c ../src/tlb.h ../src/vmm.c ../src/vmm.h)
TARGET_LINK_LIBRARIES(check_tests pthread check_pic pthread rt m subunit libelf.a)