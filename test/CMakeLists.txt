#
# Test
#

# Add the test executable

add_executable(ready_queue_tests ready_queue_tests.c)

target_include_directories(ready_queue_tests PRIVATE ../src)
target_link_libraries(ready_queue_tests PRIVATE scheduler_lib check_pic rt m subunit Threads::Threads)

# Copy the test files to the build directory

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/autograder.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_12_p.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_30_p.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_50_p.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_quick.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Add tests

add_test(NAME autograder COMMAND python3 autograder.py)