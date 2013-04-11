cmake_minimum_required (VERSION 2.8)

set(TEST_SOURCES
  ./UnitTests.cc
  $<TARGET_OBJECTS:boldhumanoid_objects>
)

#
## Target for making google-test
# TODO make this compile in correct place when using out-of-source compilation
externalproject_add(googletests
                    PREFIX google-test
		    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/google-test
		    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/google-test
		    DOWNLOAD_COMMAND ""
		    UPDATE_COMMAND ""
		    INSTALL_COMMAND "")

#
## Target for running tests. Custom targets are always run
#
add_custom_target(runtests ALL ./unittests DEPENDS unittests)

externalproject_get_property(googletests binary_dir)
set(GOOGLETESTS_BINARY_DIR ${binary_dir})

#
## Include directories
#
include_directories(${BOLDHUMANOID_INCLUDE_DIRS} google-test/include)

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
target_link_libraries(unittests ${BOLDHUMANOID_LINK_LIBRARIES} gtest)

#
## Additional compile flags
#
set_target_properties(unittests PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})


add_dependencies(runtests googletests)
