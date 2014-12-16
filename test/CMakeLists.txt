cmake_minimum_required (VERSION 2.8)

set(TEST_SOURCES
  UnitTests.cc
  google-test/src/gtest-all.cc
  AgentPositionTests.cc
  BlobTests.cc
  BodyStateTests.cc
  Bounds2iTests.cc
  BoundsTests.cc
  BufferReaderTests.cc
  CameraModelTests.cc
  CM730Tests.cc
  ColourTests.cc
  ConditionalsTests.cc
  ConsumerQueueThreadTests.cc
  CppTests.cc
  DarwinBodyModelTests.cc
  DistributionTrackerTests.cc
  EigenTests.cc
  HalfHullBuilderTests.cc
  LabelTeacherTests.cc
  HistogramPixelLabelTests.cc
  IncrementalRegressionTests.cc
  IntegralImageTests.cc
  JointIdTests.cc
  JointSelectionTests.cc
  LinearSmootherTests.cc
  LineJunctionFinderTests.cc
  LineSegmentTests.cc
  MathTests.cc
  MetaTests.cc
  MotionScriptRunnerTests.cc
  MotionTaskSchedulerTests.cc
  MX28AlarmTests.cc
  MX28Tests.cc
  ParticleFilterTests.cc
  Polygon2Tests.cc
  RangeTests.cc
  RunTests.cc
  SchmittTriggerTests.cc
  SequentialTimerTests.cc
  SignalTests.cc
  SpatialiserTest.cc
  StateTests.cc
  StationaryMapTests.cc
  StatsTests.cc
  ThreadIdTests.cc
  ThreadTests.cc
  UDPSocketTests.cc
  VisualCortexTests.cc
  WindowFunctionTests.cc
  $<TARGET_OBJECTS:boldhumanoid_objects>
)

#
## Target for running tests. Custom targets are always run
#
add_custom_target(runtests ALL ./unittests DEPENDS unittests)

#
## Include directories
#
include_directories(SYSTEM google-test/include)
include_directories(${BOLDHUMANOID_INCLUDE_DIRS} google-test)

#
## Link directories
#
link_directories(${DARWINDIR}/Linux/lib ${LIBWEBSOCKETDIR}/lib)

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
