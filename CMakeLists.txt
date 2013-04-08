cmake_minimum_required (VERSION 2.8)
project (BoldHumanoid)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

set(BUILDRUN_UNIT_TEST ON CACHE BOOL "Choose whether to build and run unit tests")

include(ExternalProject)
include(AddPcih)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(SigC++ REQUIRED)

set(DARWINDIR ${CMAKE_SOURCE_DIR}/robotis)
set(JOYSTICKDIR ${CMAKE_SOURCE_DIR}/joystick)
set(LIBWEBSOCKETDIR ${CMAKE_SOURCE_DIR}/libwebsockets)
set(RAPIDJSONDIR ${CMAKE_SOURCE_DIR}/rapidjson)

#
## Compile darwin lib as an external project
#
ExternalProject_Add(darwin_lib
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  SOURCE_DIR ${DARWINDIR}
  BINARY_DIR ${DARWINDIR}/Linux/build
  BUILD_COMMAND "make"
  INSTALL_DIR ${DARWINDIR}/Linux/lib
  INSTALL_COMMAND "")

ExternalProject_Add_Step(darwin_lib force_build
   COMMAND ${CMAKE_COMMAND} -E echo "Building libdarwin"
   DEPENDERS build
   ALWAYS 1
)
#
## Compile libwebsockets as an external project
#
ExternalProject_Add(libwebsockets_lib
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${LIBWEBSOCKETDIR}
  BINARY_DIR ${LIBWEBSOCKETDIR}
  INSTALL_COMMAND ""
)

#
# All boldhumanoid sources
#
set(BOLDHUMANOID_SOURCES
./Agent/Agent.cc
./Agent/approachBall.cc
./Agent/circleBall.cc
./Agent/init.cc
./Agent/initCamera.cc
./Agent/initBody.cc
./Agent/lookAt.cc
./Agent/lookAtBall.cc
./Agent/lookAtGoal.cc
./Agent/lookForBall.cc
./Agent/lookForGoal.cc
./Agent/preKickLook.cc
./Agent/processInputCommands.cc
./Agent/readSubBoardData.cc
./Agent/run.cc
./Agent/standUpIfFallen.cc
./Agent/think.cc
./AgentModel/initBody.cc
./AgentModel/updatePosture.cc
./Camera/Camera.cc
./Camera/capture.cc
./Camera/getControlValue.cc
./Camera/initMemoryMapping.cc
./Camera/listControls.cc
./Camera/listFormats.cc
./Camera/open.cc
./Camera/pixelFormatRequestSize.cc
./Camera/startCapture.cc
./Camera/stopCapture.cc
./CameraModel/directionForPixel.cc
./Colour/bgr2hsv.cc
./Colour/hsv2bgr.cc
./Colour/hsvRange.cc
./Colour/operator_ins.cc
./Colour/YCbCr.cc
./Control/create.cc
./Control/handleRequest.cc
./Control/staticfields.cc
./Control/writeState.cc
./CM730Snapshot/init.cc
./Debugger/debugger.cc
./DataStreamer/callbackAgentModel.cc
./DataStreamer/callbackCamera.cc
./DataStreamer/callbackGameState.cc
./DataStreamer/callbackHttp.cc
./DataStreamer/callbackTiming.cc
./DataStreamer/close.cc
./DataStreamer/DataStreamer.cc
./DataStreamer/getDebugControls.cc
./DataStreamer/initialise.cc
./DataStreamer/processCameraCommand.cc
./DataStreamer/registerControls.cc
./DataStreamer/sendCameraControls.cc
./DataStreamer/sendImageBytes.cc
./DataStreamer/shouldProvideImage.cc
./DataStreamer/streamImage.cc
./DataStreamer/update.cc
./DataStreamer/writeJson.cc
./GameController/GameControllerReceiver.cc
./geometry/Bounds2i.cc
./geometry/LineSegment2i.cc
./HoughLineAccumulator/houghlineaccumulator.cc
./HoughLineExtractor/findLines.cc
./HoughLineExtractor/findMaxLine.cc
./ImageLabeller/ImageLabeller.cc
./ImagePassHandler/BlobDetectPass/detectBlobs.cc
./ImagePassHandler/BlobDetectPass/runSetToBlob.cc
./ImagePassHandler/BlobDetectPass/BlobDetectPass.cc
./LineFinder/RandomPairLineFinder/findLineSegments.cc
./LineFinder/MaskWalkLineFinder/findLineSegments.cc
./LineFinder/MaskWalkLineFinder/MaskWalkLineFinder.cc
./LineFinder/MaskWalkLineFinder/rebuild.cc
./LUTBuilder/buildLookUpTable.cc
./Math/math.cc
./MX28Snapshot/init.cc
./Option/ActionOption/hasTerminated.cc
./Option/ActionOption/runPolicy.cc
./Option/FSMOption/runPolicy.cc
./PixelFilterChain/applyFilters.cc
./PixelLabel/fromConfig.cc
./PixelLabel/pixellabel.cc
./Spatialiser/findGroundPointForPixel.cc
./VisualCortex/VisualCortex.cc
./VisualCortex/initialise.cc
./VisualCortex/integrateImage.cc
./VisualCortex/streamDebugImage.cc
./WorldModel/initialise.cc
${JOYSTICKDIR}/joystick.cc
)

#
## Internal header files to be pre-compiled
#
set(INTERNAL_HEADERS
./Agent/agent.ih
./AgentModel/agentmodel.ih
./Camera/camera.ih
./Colour/colour.ih
./Control/control.ih
./DataStreamer/datastreamer.ih
./HoughLineAccumulator/houghlineaccumulator.ih
./ImagePassHandler/BlobDetectPass/blobdetectpass.ih
./LUTBuilder/lutbuilder.ih
./PixelFilterChain/pixelfilterchain.ih
./VisualCortex/visualcortex.ih)

#
## Include directories
#
set(BOLDHUMANOID_INCLUDE_DIRS
  ${DARWINDIR}/Linux/include
  ${DARWINDIR}/Framework/include
  ${JOYSTICKDIR}
  ${LIBWEBSOCKETDIR}/lib
  ${EIGEN3_INCLUDE_DIR}
  ${SigC++_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIR}
  ${RAPIDJSONDIR}/include
  )

include_directories(${BOLDHUMANOID_INCLUDE_DIRS})

#
## Link directories
#
set(BOLDHUMANOID_LINK_DIRS ${DARWINDIR}/Linux/lib ${LIBWEBSOCKETDIR}/lib)
link_directories(${BOLDHUMANOID_LINK_DIRS})

#
## Reusable object files target
#
add_library(boldhumanoid_objects OBJECT
  ${BOLDHUMANOID_SOURCES}
)

#
## Main executable
#
add_executable(boldhumanoid
  ./main.cc
  $<TARGET_OBJECTS:boldhumanoid_objects>
)

#
## Link executable to libraries
#
set(BOLDHUMANOID_LINK_LIBRARIES ${OpenCV_LIBS} ${SigC++_LIBRARIES} darwin websockets pthread rt)
target_link_libraries(boldhumanoid ${BOLDHUMANOID_LINK_LIBRARIES})

#
## Additional compile flags
# TODO: fix things so we can use Eigen vectorization
#
set(BOLDHUMANOID_COMPILE_FLAGS "-std=c++0x -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -Wall -Wno-sign-compare -Wno-unused-local-typedefs")
set_target_properties(boldhumanoid_objects PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})
set_target_properties(boldhumanoid PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

#
## Targets for creating precompiled internal headers
#
foreach(ih ${INTERNAL_HEADERS})
  add_pcih(boldhumanoid_objects ${ih})
endforeach(ih)


#
## Target dependencies
#
add_dependencies(boldhumanoid darwin_lib)
add_dependencies(boldhumanoid libwebsockets_lib)

#
## Build and run test
#
if (BUILDRUN_UNIT_TEST)
  add_subdirectory(test)
endif (BUILDRUN_UNIT_TEST)

#
## Build tools
#
add_subdirectory(tools)
