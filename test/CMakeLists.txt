cmake_minimum_required (VERSION 2.8)

set(TEST_SOURCES
  ./UnitTests.cc
  ./google-test/src/gtest-all.cc
  $<TARGET_OBJECTS:boldhumanoid_objects>
)

#
## Target for running tests. Custom targets are always run
#
add_custom_target(runtests ALL ./unittests DEPENDS unittests)

#
## Include directories
#
include_directories(${BOLDHUMANOID_INCLUDE_DIRS} google-test/include google-test)

#
## Link directories
#
set(BOLDHUMANOID_LINK_DIRS ${DARWINDIR}/Linux/lib ${LIBWEBSOCKETDIR}/lib)
link_directories(${BOLDHUMANOID_LINK_DIRS} ${GOOGLETESTS_BINARY_DIR})

#
## Target
#
add_executable(unittests
  ${TEST_SOURCES}
)

#
## Link executable to libraries
#
target_link_libraries(unittests ${BOLDHUMANOID_LINK_LIBRARIES})

#
## Additional compile flags
#
set_target_properties(unittests PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})


add_dependencies(runtests googletests)
