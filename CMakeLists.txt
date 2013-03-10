cmake_minimum_required (VERSION 2.8)
project (BoldHumanoid)

include(/usr/share/cmake-2.8/Modules/ExternalProject.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

find_package(Eigen3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(SigC++ REQUIRED)

set(DARWINDIR ${CMAKE_SOURCE_DIR}/robotis)
set(VISIONDIR ${CMAKE_SOURCE_DIR}/vision)
set(JOYSTICKDIR ${CMAKE_SOURCE_DIR}/joystick)
set(LIBWEBSOCKETDIR ${CMAKE_SOURCE_DIR}/libwebsockets)
set(RAPIDJSONDIR ${CMAKE_SOURCE_DIR}/rapidjson)

# Macro for adding a precompiled internal header file
macro(add_pcih _target _ih_path)
  string(REPLACE ".ih" ".ih.gch" _pcih_path ${_ih_path})
  get_filename_component(_ih_name ${_ih_path} NAME_WE)

  set(_ih_full_path ${CMAKE_CURRENT_SOURCE_DIR}/${_ih_path})
  set(_pcih_full_path ${CMAKE_CURRENT_BINARY_DIR}/${_pcih_path})

  get_target_property(_COMPILE_FLAGS ${_target} COMPILE_FLAGS)

  get_directory_property(_directory_flags INCLUDE_DIRECTORIES)
  foreach(item ${_directory_flags})
    list(APPEND _COMPILE_FLAGS "-I${item}")
  endforeach(item)
 
  get_directory_property(_directory_flags DEFINITIONS)
  list(APPEND _COMPILE_FLAGS ${_directory_flags})

  separate_arguments(_COMPILE_FLAGS)

  add_custom_command(OUTPUT ${_pcih_full_path}
                     COMMAND ${CMAKE_CXX_COMPILER} -x c++-header ${_COMPILE_FLAGS} -c ${_ih_full_path} -o ${_pcih_full_path}
                     IMPLICIT_DEPENDS CXX ${_ih_full_path})
  
  set(_pcih_target "${_ih_name}_pcih")
  add_custom_target(${_pcih_target} DEPENDS ${_pcih_full_path})
  add_dependencies(${_target} ${_pcih_target})
              
endmacro(add_pcih)

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

# All sources
set(BOLDHUMANOID_SOURCES
./main.cc
./Agent/Agent.cc
./Agent/run.cc
./Agent/init.cc
./Agent/initCamera.cc
./Agent/think.cc
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
./CM730Snapshot/init.cc
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
./DataStreamer/sendCameraControls.cc
./DataStreamer/sendStreamLabels.cc
./DataStreamer/sendImage.cc
./GameController/GameControllerReceiver.cc
./MX28Snapshot/init.cc
./VisualCortex/VisualCortex.cc
./VisualCortex/initialise.cc
./VisualCortex/integrateImage.cc
./VisualCortex/pixelLabelFromConfig.cc
./WorldModel/initialise.cc
${VISIONDIR}/Camera/Camera.cc
${VISIONDIR}/Camera/capture.cc
${VISIONDIR}/Camera/fillControlMenuItems.cc
${VISIONDIR}/Camera/getControlValue.cc
${VISIONDIR}/Camera/initMemoryMapping.cc
${VISIONDIR}/Camera/listControls.cc
${VISIONDIR}/Camera/listFormats.cc
${VISIONDIR}/Camera/open.cc
${VISIONDIR}/Camera/pixelFormatRequestSize.cc
${VISIONDIR}/Camera/setControlValue.cc
${VISIONDIR}/Camera/startCapture.cc
${VISIONDIR}/Camera/stopCapture.cc
${VISIONDIR}/Colour/bgr2hsv.cc
${VISIONDIR}/Colour/hsv2bgr.cc
${VISIONDIR}/Colour/hsvRange.cc
${VISIONDIR}/Colour/YCbCr.cc
${VISIONDIR}/geometry/Bounds2i.cc
${VISIONDIR}/geometry/LineSegment2i.cc
${VISIONDIR}/HoughLineAccumulator/houghlineaccumulator.cc
${VISIONDIR}/HoughLineExtractor/findLines.cc
${VISIONDIR}/HoughLineExtractor/findMaxLine.cc
${VISIONDIR}/ImageLabeller/ImageLabeller.cc
${VISIONDIR}/ImagePassHandler/BlobDetectPass/detectBlobs.cc
${VISIONDIR}/ImagePassHandler/BlobDetectPass/runSetToBlob.cc
${VISIONDIR}/LineFinder/find.cc
${VISIONDIR}/LUTBuilder/buildLookUpTable.cc
${VISIONDIR}/PixelFilterChain/applyFilters.cc
${VISIONDIR}/PixelLabel/pixellabel.cc
${JOYSTICKDIR}/joystick.cc
)

# Internal header files to be compiles
set(INTERNAL_HEADERS
./vision/PixelFilterChain/pixelfilterchain.ih
./vision/Camera/camera.ih
./vision/HoughLineAccumulator/houghlineaccumulator.ih
./vision/LUTBuilder/lutbuilder.ih
./DataStreamer/datastreamer.ih
./Agent/agent.ih
./VisualCortex/visualcortex.ih)

include_directories(
${DARWINDIR}/Linux/include
${DARWINDIR}/Framework/include
${VISIONDIR}
${JOYSTICKDIR}
${LIBWEBSOCKETDIR}/lib
${EIGEN3_INCLUDE_DIR}
${SigC++_INCLUDE_DIRS}
${OpenCV_INCLUDE_DIR}
${RAPIDJSONDIR}/include)

add_executable(boldhumanoid
  ${BOLDHUMANOID_SOURCES}
)

target_link_libraries(boldhumanoid ${OpenCV_LIBS} ${SigC++_LIBRARIES} darwin websockets pthread rt)

# TODO: fix things so we can use Eigen vectorization
set_target_properties(boldhumanoid PROPERTIES COMPILE_FLAGS "-std=c++0x -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT")

# Additional target for creating precompiled internal headers
foreach(ih ${INTERNAL_HEADERS})
  add_pcih(boldhumanoid ${ih})
endforeach(ih)

add_dependencies(boldhumanoid darwin_lib)
add_dependencies(boldhumanoid libwebsockets_lib)
