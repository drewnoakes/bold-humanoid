cmake_minimum_required (VERSION 2.8)
project (BoldHumanoid)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

set(BUILDRUN_UNIT_TEST ON CACHE BOOL "Choose whether to build and run unit tests")
set(PRECOMPILE_HEADERS ON CACHE BOOL "Choose whether to precompile headers")
set(UPLOAD_SCP_PATH "" CACHE STRING "Path to use in upload scp command. Format: user@server:path")

include(ExternalProject)
include(AddPcih)
include(AddClass)

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
  CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  INSTALL_COMMAND ""
)

#
## Boldhumanoid sources
#
# Only add sources here that are not in class directory structure! Add classes separately below
#
set(BOLDHUMANOID_SOURCES
  ${JOYSTICKDIR}/joystick.cc
)

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
## Additional compile flags
#
set(BOLDHUMANOID_COMPILE_FLAGS "-std=c++0x -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -Wall -Wno-sign-compare -Wno-unused-local-typedefs -Wno-enum-compare")

add_class(BOLDHUMANOID
  ./Agent/Agent.cc
  ./Agent/approachBall.cc
  ./Agent/circleBall.cc
  ./Agent/initCamera.cc
  ./Agent/initMotionManager.cc
  ./Agent/lookAt.cc
  ./Agent/lookAtBall.cc
  ./Agent/lookAtGoal.cc
  ./Agent/lookForBall.cc
  ./Agent/lookForGoal.cc
  ./Agent/preKickLook.cc
  ./Agent/processInputCommands.cc
  ./Agent/readSubBoardData.cc
  ./Agent/registerStateTypes.cc
  ./Agent/run.cc
  ./Agent/standUpIfFallen.cc
  ./Agent/think.cc
)

add_class(BOLDHUMANOID
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
)

add_class(BOLDHUMANOID
  ./CameraModel/directionForPixel.cc
  ./CameraModel/pixelForDirection.cc
)

add_class(BOLDHUMANOID
  ./Colour/bgr2hsv.cc
  ./Colour/hsv2bgr.cc
  ./Colour/hsvRange.cc
  ./Colour/operator_ins.cc
  ./Colour/YCbCr.cc
)

add_class(BOLDHUMANOID
  ./Control/create.cc
  ./Control/handleRequest.cc
  ./Control/staticfields.cc
  ./Control/writeState.cc
)

add_class(BOLDHUMANOID
  ./CM730Snapshot/CM730Snapshot.cc
)

add_class(BOLDHUMANOID
  ./Debugger/debugger.cc
)

add_class(BOLDHUMANOID
  ./DataStreamer/callbackCamera.cc
  ./DataStreamer/callbackHttp.cc
  ./DataStreamer/callbackState.cc
  ./DataStreamer/callbackTiming.cc
  ./DataStreamer/close.cc
  ./DataStreamer/DataStreamer.cc
  ./DataStreamer/getDebugControls.cc
  ./DataStreamer/processCameraCommand.cc
  ./DataStreamer/registerControls.cc
  ./DataStreamer/sendCameraControls.cc
  ./DataStreamer/sendImageBytes.cc
  ./DataStreamer/shouldProvideImage.cc
  ./DataStreamer/streamImage.cc
  ./DataStreamer/update.cc
  ./DataStreamer/writeJson.cc
)

add_class(BOLDHUMANOID
  ./GameStateReceiver/GameStateReceiver.cc
  ./GameStateReceiver/receive.cc
)

add_class(BOLDHUMANOID
  ./geometry/Bounds2i.cc
  ./geometry/LineSegment2i.cc
)

add_class(BOLDHUMANOID
  ./HoughLineAccumulator/houghlineaccumulator.cc
)

add_class(BOLDHUMANOID
  ./HoughLineExtractor/findLines.cc
  ./HoughLineExtractor/findMaxLine.cc
)

add_class(BOLDHUMANOID
  ./ImageLabeller/ImageLabeller.cc
)

add_class(BOLDHUMANOID
  ./ImagePassHandler/BlobDetectPass/detectBlobs.cc
  ./ImagePassHandler/BlobDetectPass/runSetToBlob.cc
  ./ImagePassHandler/BlobDetectPass/BlobDetectPass.cc
)

add_class(BOLDHUMANOID
  ./LineFinder/RandomPairLineFinder/findLineSegments.cc
)

add_class(BOLDHUMANOID
  ./LineFinder/MaskWalkLineFinder/findLineSegments.cc
  ./LineFinder/MaskWalkLineFinder/MaskWalkLineFinder.cc
  ./LineFinder/MaskWalkLineFinder/rebuild.cc
  ./LineFinder/MaskWalkLineFinder/walkLine.cc
)

add_class(BOLDHUMANOID
  ./LUTBuilder/buildLookUpTable.cc
)

add_class(BOLDHUMANOID
  ./Math/math.cc
)

add_class(BOLDHUMANOID
  ./MX28Snapshot/MX28Snapshot.cc
)

add_class(BOLDHUMANOID
  ./Option/ActionOption/hasTerminated.cc
  ./Option/ActionOption/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/ApproachBall/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/FSMOption/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAround/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAtBall/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/Stand/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./OptionTree/run.cc
)

add_class(BOLDHUMANOID
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildTree.cc
)

add_class(BOLDHUMANOID
  ./PixelFilterChain/applyFilters.cc
)

add_class(BOLDHUMANOID
  ./PixelLabel/fromConfig.cc
  ./PixelLabel/pixellabel.cc
)

add_class(BOLDHUMANOID
  ./Spatialiser/findGroundPointForPixel.cc
  ./Spatialiser/updateCameraToAgent.cc
  ./Spatialiser/updateAgentToWorld.cc
)

add_class(BOLDHUMANOID
  ./StateObject/AgentFrameState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/AlarmState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/BodyState/initBody.cc
  ./StateObject/BodyState/updatePosture.cc
  ./StateObject/BodyState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/CameraFrameState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/GameState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/HardwareState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/OptionTreeState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/WorldFrameState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./VisualCortex/VisualCortex.cc
  ./VisualCortex/integrateImage.cc
  ./VisualCortex/streamDebugImage.cc
)

add_class(BOLDHUMANOID
  ./FieldMap/FieldMap.cc
)

#
## Reusable object files target
#
add_library(boldhumanoid_objects OBJECT
  ${BOLDHUMANOID_SOURCES}
)

foreach(pch_target ${BOLDHUMANOID_PCH_TARGETS})
  add_dependencies(boldhumanoid_objects ${pch_target})
endforeach(pch_target)

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
set_target_properties(boldhumanoid_objects PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})
set_target_properties(boldhumanoid PROPERTIES COMPILE_FLAGS ${BOLDHUMANOID_COMPILE_FLAGS})

#
## Target dependencies
#
add_dependencies(boldhumanoid darwin_lib)
add_dependencies(boldhumanoid libwebsockets_lib)

if(UPLOAD_SCP_PATH)
  add_custom_command(OUTPUT upload.stamp
    COMMAND scp -C $<TARGET_FILE:boldhumanoid> ${UPLOAD_SCP_PATH}
    COMMAND scp -C ${CMAKE_SOURCE_DIR}/*.ini ${UPLOAD_SCP_PATH}
    COMMAND rsync -rP ${CMAKE_SOURCE_DIR}/www ${UPLOAD_SCP_PATH}
    COMMAND touch upload.stamp
    DEPENDS boldhumanoid)

  add_custom_target(upload DEPENDS upload.stamp)
else(UPLOAD_SCP_PATH)
  message(WARNING "You have not set a path for uploading the binary; 'upload' target not generated")
endif(UPLOAD_SCP_PATH)


set(_command find ${CMAKE_SOURCE_DIR} -regextype posix-egrep -regex ".*((hh)|(cc))$" -print | etags --declarations -o ${CMAKE_SOURCE_DIR}/TAGS -)
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/TAGS
  COMMAND ${_command}
  IMPLICIT_DEPENDS ${BOLDHUMANOID_SORCES}
  VERBATIM
  )
add_custom_target(tags DEPENDS ${CMAKE_SOURCE_DIR}/TAGS)

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
