cmake_minimum_required(VERSION 3.11)
project(check_tests C)

set(CMAKE_C_STANDARD 11)

####################################################################################
# <Libcheck libs>                                                                    #
####################################################################################

add_executable(check_tests checks.c check_utils.h ../src/main.c ../src/main.h call_by_string.c call_by_string.h)
TARGET_LINK_LIBRARIES(check_tests pthread check_pic pthread rt m subunit libelf.a)
#add_executable(TP0_test template.c)
