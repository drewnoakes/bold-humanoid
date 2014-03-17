cmake_minimum_required (VERSION 2.8)

macro(add_test _UNITTEST_NAME)
  set(UNITTEST_NAME ${_UNITTEST_NAME})
  set(UNITTEST_SOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/unittest_${UNITTEST_NAME}.cc")
  set(UNITTEST_EXECUTABLE "unittest_${UNITTEST_NAME}")

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/unittest.cc.in" ${UNITTEST_SOURCE_FILE} @ONLY)

  add_executable(${UNITTEST_EXECUTABLE}
    ${UNITTEST_SOURCE_FILE}
    )

  add_custom_target(run_${UNITTEST_NAME} ALL ./${UNITTEST_EXECUTABLE} DEPENDS ${UNITTEST_EXECUTABLE})

  target_link_libraries(${UNITTEST_EXECUTABLE} ${BOLDHUMANOID_LINK_LIBRARIES} googletest bold)

  set_target_properties(${UNITTEST_EXECUTABLE} PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})
  set_target_properties(${UNITTEST_EXECUTABLE} PROPERTIES LINK_FLAGS "-Wl,--gc-sections")
endmacro(add_test)

add_library(googletest
    ./google-test/src/gtest-all.cc
)

#
## Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${BOLDHUMANOID_INCLUDE_DIRS} google-test/include google-test)

#
## Link directories
#
set(BOLDHUMANOID_LINK_DIRS ${DARWINDIR}/Linux/lib ${LIBWEBSOCKETDIR}/lib)
link_directories(${BOLDHUMANOID_LINK_DIRS} ${GOOGLETESTS_BINARY_DIR})


#add_dependencies(runtests googletests)

add_test("AgentPositionTests")
add_test("BlobTests")
add_test("BodyStateTests")
add_test("Bounds2iTests")
add_test("BoundsTests")
add_test("CameraModelTests")
add_test("CM730Tests")
add_test("ColourTests")
add_test("ConditionalsTests")
add_test("ConsumerQueueThreadTests")
add_test("CppTests")
add_test("DistributionTrackerTests")
add_test("EigenTests")
add_test("HalfHullBuilderTests")
add_test("IntegralImageTests")
add_test("JointSelectionTests")
add_test("LinearSmootherTests")
add_test("LineSegmentTests")
add_test("MathTests")
add_test("MetaTests")
add_test("MotionScriptRunnerTests")
add_test("MX28AlarmTests")
add_test("MX28Tests")
add_test("ParticleFilterTests")
add_test("Polygon2Tests")
add_test("RangeTests")
add_test("RunTests")
add_test("SchmittTriggerTests")
add_test("SequentialTimerTests")
add_test("SignalTests")
add_test("SpatialiserTest")
add_test("StateTests")
add_test("StatsTests")
add_test("ThreadIdTests")
add_test("ThreadTests")
add_test("UDPSocketTests")
add_test("VisualCortexTests")
