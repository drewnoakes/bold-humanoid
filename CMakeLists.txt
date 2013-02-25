cmake_minimum_required (VERSION 2.8)
project (BoldHumanoid)

include(/usr/share/cmake-2.8/Modules/ExternalProject.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

find_package(Eigen3 REQUIRED)
find_package(Eigen3 REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)
find_package(SigC++ REQUIRED)

set(DARWINDIR ${CMAKE_SOURCE_DIR}/robotis)
set(VISIONDIR ${CMAKE_SOURCE_DIR}/vision)
set(JOYSTICKDIR ${CMAKE_SOURCE_DIR}/joystick)
set(LIBWEBSOCKETDIR ${CMAKE_SOURCE_DIR}/libwebsockets)

# Compile darwin lib as an external project
ExternalProject_Add(darwin_lib
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  SOURCE_DIR ${DARWINDIR}
  BINARY_DIR ${DARWINDIR}/Linux/build
  BUILD_COMMAND "make"
  INSTALL_DIR ${DARWINDIR}/Linux/lib
  INSTALL_COMMAND "")

# Compile libwebsockets as an externl project
ExternalProject_Add(libwebsockets_lib
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${LIBWEBSOCKETDIR}
  BINARY_DIR ${LIBWEBSOCKETDIR}
#  BUILD_COMMAND "make"
#  INSTALL_DIR ${DARWINDIR}/Linux/lib
  INSTALL_COMMAND ""
)

link_directories(${DARWINDIR}/Linux/lib ${LIBWEBSOCKETDIR}/lib)

  
add_executable(boldhumanoid
./main.cc
./Agent/Agent.cc
./Agent/run.cc
./Agent/init.cc
./Agent/initCamera.cc
./Agent/think.cc
./Agent/processImage.cc
./Agent/processInputCommands.cc
./Agent/standUpIfFallen.cc
./Agent/lookForBall.cc
./Agent/lookAtBall.cc
./Agent/approachBall.cc
./Agent/circleBall.cc
./Agent/lookForGoal.cc
./Agent/lookAtGoal.cc
./Agent/lookAt.cc
./Agent/preKickLook.cc
./Agent/readSubBoardData.cc
./Debugger/debugger.cc
./DataStreamer/callbackAgentModel.cc
./DataStreamer/callbackCamera.cc
./DataStreamer/callbackGameState.cc
./DataStreamer/callbackHttp.cc
./DataStreamer/callbackTiming.cc
./DataStreamer/close.cc
./DataStreamer/DataStreamer.cc
./DataStreamer/init.cc
./DataStreamer/streamImage.cc
./DataStreamer/update.cc
./MX28Snapshot/init.cc
./GameController/GameControllerReceiver.cc
./CM730Snapshot/init.cc
${VISIONDIR}/BlobDetector/detectBlobs.cc
${VISIONDIR}/BlobDetector/runLengthEncode.cc
${VISIONDIR}/BlobDetector/runSetToBlob.cc
${VISIONDIR}/PixelFilterChain/applyFilters.cc
${VISIONDIR}/Camera/Camera.cc
${VISIONDIR}/Camera/capture.cc
${VISIONDIR}/Camera/open.cc
${VISIONDIR}/Camera/startCapture.cc
${VISIONDIR}/Camera/listControls.cc
${VISIONDIR}/Camera/stopCapture.cc
${VISIONDIR}/Camera/fillControlMenuItems.cc
${VISIONDIR}/Camera/setControlValue.cc
${VISIONDIR}/Camera/listFormats.cc
${VISIONDIR}/Camera/getControlValue.cc
${VISIONDIR}/Camera/pixelFormatRequestSize.cc
${VISIONDIR}/Camera/initMemoryMapping.cc
${VISIONDIR}/LUTBuilder/bgr2hsv.cc
${VISIONDIR}/LUTBuilder/buildBGRFromHSVRanges.cc
${JOYSTICKDIR}/joystick.cc
)


include_directories(
${DARWINDIR}/Linux/include
${DARWINDIR}/Framework/include
${VISIONDIR}
${JOYSTICKDIR}
${EIGEN3_INCLUDE_DIR}
${SigC++_INCLUDE_DIRS}
${OpenCV_INCLUDE_DIR})

target_link_libraries(boldhumanoid ${OpenCV_LIBS} ${SigC++_LIBRARIES} darwin websockets pthread rt)

# TODO: fix things so we can use Eigen vectorization
set(CMAKE_CXX_FLAGS "-std=c++0x -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT")


add_dependencies(boldhumanoid darwin_lib)
add_dependencies(boldhumanoid libwebsockets_lib)

