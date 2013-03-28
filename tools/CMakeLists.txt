cmake_minimum_required (VERSION 2.8)

set (PASSTEST_SOURCES
../geometry/Bounds2i.cc
../geometry/LineSegment2i.cc
../Camera/Camera.cc
../Camera/capture.cc
../Camera/initMemoryMapping.cc
../Camera/getControlValue.cc
../Camera/listControls.cc
../Camera/listFormats.cc
../Camera/open.cc
../Camera/pixelFormatRequestSize.cc
../Camera/startCapture.cc
../Camera/stopCapture.cc
../Control/create.cc
../Colour/bgr2hsv.cc
../Colour/hsv2bgr.cc
../Colour/hsvRange.cc
../Colour/YCbCr.cc
../Control/handleRequest.cc
../Control/staticfields.cc
../Control/writeState.cc
../HoughLineAccumulator/houghlineaccumulator.cc
../HoughLineExtractor/findLines.cc
../HoughLineExtractor/findMaxLine.cc
../ImageLabeller/ImageLabeller.cc
../ImagePassHandler/BlobDetectPass/BlobDetectPass.cc
../ImagePassHandler/BlobDetectPass/detectBlobs.cc
../ImagePassHandler/BlobDetectPass/runSetToBlob.cc
../LineFinder/MaskWalkLineFinder/findLineSegments.cc
../LineFinder/MaskWalkLineFinder/initialise.cc
../LineFinder/MaskWalkLineFinder/MaskWalkLineFinder.cc
../LineFinder/MaskWalkLineFinder/rebuild.cc
../LineFinder/RandomPairLineFinder/findLineSegments.cc
../LUTBuilder/buildLookUpTable.cc
../PixelFilterChain/applyFilters.cc
../PixelLabel/pixellabel.cc)

add_executable(djsettest
  djsettest.cc
)

set_target_properties(djsettest PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

# todo: use actual needed sources
add_executable(passtest
  passtest.cc
  ${PASSTEST_SOURCES}
)

target_link_libraries(passtest ${BOLDHUMANOID_LINK_LIBRARIES})
set_target_properties(passtest PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

# todo: use actual needed sources
add_executable(cameratest
  cameratest.cc
  ${PASSTEST_SOURCES}
)

target_link_libraries(cameratest ${BOLDHUMANOID_LINK_LIBRARIES})
set_target_properties(cameratest PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

# todo: use actual needed sources
add_executable(hough
  hough.cc
  ${PASSTEST_SOURCES}
)

target_link_libraries(hough ${BOLDHUMANOID_LINK_LIBRARIES})
set_target_properties(hough PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})
