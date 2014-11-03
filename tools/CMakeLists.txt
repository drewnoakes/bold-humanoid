cmake_minimum_required (VERSION 2.8)

find_package(OpenCV REQUIRED highgui)

set (PASSTEST_SOURCES
  $<TARGET_OBJECTS:boldhumanoid_objects>
)

#add_executable(djsettest
#  djsettest.cc
#)
#set_target_properties(djsettest PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

# todo: use actual needed sources
add_executable(passtest
  passtest.cc
  ${PASSTEST_SOURCES}
)
target_link_libraries(passtest ${BOLDHUMANOID_LINK_LIBRARIES} highgui)
set_target_properties(passtest PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

# todo: use actual needed sources
#add_executable(cameratest
#  cameratest.cc
#  ${PASSTEST_SOURCES}
#)
#target_link_libraries(cameratest ${BOLDHUMANOID_LINK_LIBRARIES})
#set_target_properties(cameratest PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

# todo: use actual needed sources
#add_executable(hough
#  hough.cc
#  ${PASSTEST_SOURCES}
#)
#target_link_libraries(hough ${BOLDHUMANOID_LINK_LIBRARIES})
#set_target_properties(hough PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})
