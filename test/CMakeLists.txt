#
# Test
#

# Copy the test files to the build directory

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/autograder.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/autograder_config.yaml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Add tests

add_test(NAME autograder COMMAND python3 autograder.py)