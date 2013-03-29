cmake_minimum_required (VERSION 2.8)

set(TEST_SOURCES
  ./UnitTests.cc
  ../AgentModel/initBody.cc
  ../AgentModel/updatePosture.cc
  ../CameraModel/directionForPixel.cc
  ../Colour/bgr2hsv.cc
  ../Colour/hsv2bgr.cc
  ../Colour/hsvRange.cc
  ../Colour/operator_ins.cc
  ../Colour/YCbCr.cc
  ../Control/create.cc
  ../Control/handleRequest.cc
  ../Control/staticfields.cc
  ../Control/writeState.cc
  ../geometry/Bounds2i.cc
  ../geometry/LineSegment2i.cc
  ../HoughLineAccumulator/houghlineaccumulator.cc
  ../HoughLineExtractor/findLines.cc
  ../HoughLineExtractor/findMaxLine.cc
  ../ImageLabeller/ImageLabeller.cc
  ../ImagePassHandler/BlobDetectPass/detectBlobs.cc
  ../ImagePassHandler/BlobDetectPass/runSetToBlob.cc
  ../LineFinder/RandomPairLineFinder/findLineSegments.cc
  ../LineFinder/MaskWalkLineFinder/initialise.cc
  ../LineFinder/MaskWalkLineFinder/findLineSegments.cc
  ../LineFinder/MaskWalkLineFinder/MaskWalkLineFinder.cc
  ../LineFinder/MaskWalkLineFinder/rebuild.cc
  ../LUTBuilder/buildLookUpTable.cc
  ../PixelFilterChain/applyFilters.cc
  ../PixelLabel/pixellabel.cc
  )

#
## Target for making google-test
#
externalproject_add(googletests
                    PREFIX google-test
		    BINARY_DIR google-test
		    SOURCE_DIR google-test
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
