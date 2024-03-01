#
# Testing
#

option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
  enable_testing()
  include(CTest)
  enable_testing()
endif ()