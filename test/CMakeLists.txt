cmake_minimum_required (VERSION 2.8)

set(TEST_SOURCES
  ./UnitTests.cc
  ../Colour/bgr2hsv.cc
  ../Colour/hsv2bgr.cc
  ../Colour/hsvRange.cc
  ../Colour/YCbCr.cc
  ../geometry/Bounds2i.cc
  ../geometry/LineSegment2i.cc
  ../HoughLineAccumulator/houghlineaccumulator.cc
  ../HoughLineExtractor/findLines.cc
  ../HoughLineExtractor/findMaxLine.cc
  ../ImageLabeller/ImageLabeller.cc
  ../ImagePassHandler/BlobDetectPass/detectBlobs.cc
  ../ImagePassHandler/BlobDetectPass/runSetToBlob.cc
  ../LineFinder/find.cc
  ../LUTBuilder/buildLookUpTable.cc
  ../PixelFilterChain/applyFilters.cc
  ../PixelLabel/pixellabel.cc
  )

#
## Include directories
#
include_directories(${BOLDHUMANOID_INCLUDE_DIRS} google-test/include)

#
## Link directories
#
set(BOLDHUMANOID_LINK_DIRS ${DARWINDIR}/Linux/lib ${LIBWEBSOCKETDIR}/lib)
link_directories(${BOLDHUMANOID_LINK_DIRS})

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

#
## Target for running tests
#
add_custom_target(RunTests ALL ./unittests DEPENDS unittests)

