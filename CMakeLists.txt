cmake_minimum_required (VERSION 2.8)
project (BoldHumanoid)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

set(BUILDRUN_UNIT_TEST ON CACHE BOOL "Choose whether to build and run unit tests")
set(BUILD_TOOLS ON CACHE BOOL "Choose whether to build tools")
set(PRECOMPILE_HEADERS ON CACHE BOOL "Choose whether to precompile headers")
set(UPLOAD_SCP_PATH "" CACHE STRING "Path to use in upload scp command. Format: user@server:path")
set(BUNDLES_PATH "" CACHE STRING "Path to use to store bundles")

include(ExternalProject)
include(AddPcih)
include(AddClass)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(SigC++ REQUIRED)

set(JOYSTICKDIR ${CMAKE_SOURCE_DIR}/joystick)
set(LIBWEBSOCKETDIR ${CMAKE_SOURCE_DIR}/libwebsockets)
set(RAPIDJSONDIR ${CMAKE_SOURCE_DIR}/rapidjson)

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
set(BOLDHUMANOID_LINK_DIRS ${LIBWEBSOCKETDIR}/lib)
link_directories(${BOLDHUMANOID_LINK_DIRS})

#
## Additional compile flags
#
set(BOLDHUMANOID_COMPILE_FLAGS "-std=c++11 -fPIC -march=atom -mtune=atom -pipe -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -Wall  -Wno-deprecated-register -Wno-sign-compare -Wno-unused-local-typedefs -Wno-enum-compare -Wno-c++11-narrowing -Wno-unknown-warning-option -Wno-deprecated")

add_class(BOLDHUMANOID
  ./Agent/Agent.cc
  ./Agent/initCamera.cc
  ./Agent/processInputCommands.cc
  ./Agent/readStaticHardwareState.cc
  ./Agent/registerStateTypes.cc
  ./Agent/run.cc
  ./Agent/setOptionTree.cc
  ./Agent/stop.cc
  ./Agent/think.cc
)

add_class(BOLDHUMANOID
  ./AgentPosition/agentWorldTransform.cc
  ./AgentPosition/worldAgentTransform.cc
)

add_class(BOLDHUMANOID
  ./Ambulator/Ambulator.cc
  ./Ambulator/setters.cc
  ./Ambulator/step.cc
)

add_class(BOLDHUMANOID
  ./BodyControl/bodycontrol.cc
)

add_class(BOLDHUMANOID
  ./Camera/Camera.cc
  ./Camera/capture.cc
  ./Camera/initMemoryMapping.cc
  ./Camera/createControls.cc
  ./Camera/createFormats.cc
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
  ./Clock/getTimestamp.cc
  ./Clock/getSeconds.cc
)

add_class(BOLDHUMANOID
  ./CM730/cm730.cc
)

add_class(BOLDHUMANOID
  ./CM730Platform/CM730Linux/cm730linux.cc
)

add_class(BOLDHUMANOID
  ./CM730Snapshot/CM730Snapshot.cc
)

add_class(BOLDHUMANOID
  ./Colour/bgr2hsv.cc
  ./Colour/hsv2bgr.cc
  ./Colour/hsvRange.cc
  ./Colour/operator_ins.cc
  ./Colour/YCbCr.cc
)

add_class(BOLDHUMANOID
  ./Config/config.cc
)

add_class(BOLDHUMANOID
  ./Debugger/debugger.cc
)

add_class(BOLDHUMANOID
  ./DataStreamer/callbackCamera.cc
  ./DataStreamer/callbackControl.cc
  ./DataStreamer/callbackHttp.cc
  ./DataStreamer/callbackState.cc
  ./DataStreamer/close.cc
  ./DataStreamer/DataStreamer.cc
  ./DataStreamer/prepareControlSyncBytes.cc
  ./DataStreamer/prepareSettingUpdateBytes.cc
  ./DataStreamer/processCommand.cc
  ./DataStreamer/streamImage.cc
  ./DataStreamer/update.cc
)

add_class(BOLDHUMANOID
  ./FieldMap/FieldMap.cc
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
  ./ImagePassHandler/FieldEdgePass/fieldedgepass.cc
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
  ./Localiser/createRandomState.cc
  ./Localiser/extract.cc
  ./Localiser/Localiser.cc
  ./Localiser/predict.cc
  ./Localiser/resample.cc
  ./Localiser/update.cc
  ./Localiser/updateSmoothedPos.cc
  ./Localiser/updateStateObject.cc
)

add_class(BOLDHUMANOID
  ./LUTBuilder/buildLookUpTable.cc
)

add_class(BOLDHUMANOID
  ./Math/createRng.cc
  ./Math/findPerpendicularVector.cc
  ./Math/intersectRayWithGroundPlane.cc
  ./Math/intersectRayWithPlane.cc
  ./Math/linePointClosestToPoint.cc
  ./Math/smallestAngleBetween.cc
)

add_class(BOLDHUMANOID
  ./MotionLoop/motionloop.cc
)

add_class(BOLDHUMANOID
  ./MotionModule/HeadModule/headmodule.cc
)

add_class(BOLDHUMANOID
  ./MotionModule/MotionScriptModule/motionscriptmodule.cc
)

add_class(BOLDHUMANOID
  ./MotionModule/WalkModule/walkmodule.cc
  ./MotionModule/WalkModule/Matrix.cpp
  ./MotionModule/WalkModule/Point.cpp
  ./MotionModule/WalkModule/Vector.cpp
)

add_class(BOLDHUMANOID
  ./MotionScript/MotionScript.cc
)

add_class(BOLDHUMANOID
  ./MotionScriptRunner/motionscriptrunner.cc
)

add_class(BOLDHUMANOID
  ./MotionTaskScheduler/add.cc
  ./MotionTaskScheduler/update.cc
)

add_class(BOLDHUMANOID
  ./MX28Snapshot/MX28Snapshot.cc
)

add_class(BOLDHUMANOID
  ./MX28/conversions.cc
)

add_class(BOLDHUMANOID
  ./Option/ApproachBall/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/CircleBall/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/FSMOption/fsmtransition.cc
  ./Option/FSMOption/runPolicy.cc
  ./Option/FSMOption/toDot.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAround/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAtBall/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAtFeet/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/LookAtGoal/runPolicy.cc
)

add_class(BOLDHUMANOID
  ./Option/MotionScriptOption/motionscriptoption.cc
)

add_class(BOLDHUMANOID
  ./Option/StopWalking/runPolicy.cc
  ./Option/StopWalking/hasTerminated.cc
)

add_class(BOLDHUMANOID
  ./OptionTree/getOption.cc
  ./OptionTree/run.cc
)

add_class(BOLDHUMANOID
  ./OptionTreeBuilder/AdHocOptionTreeBuilder/buildTree.cc
)

add_class(BOLDHUMANOID
  ./PixelFilterChain/applyFilters.cc
)

add_class(BOLDHUMANOID
  ./PixelLabel/pixellabel.cc
)

add_class(BOLDHUMANOID
  ./RobotisMotionFile/RobotisMotionFile.cc
)

add_class(BOLDHUMANOID
  ./Setting/setting.cc
  ./Setting/setting-implementations.cc
)

add_class(BOLDHUMANOID
  ./Spatialiser/findGroundPointForPixel.cc
  ./Spatialiser/findHorizonForColumn.cc
  ./Spatialiser/updateCameraToAgent.cc
  ./Spatialiser/updateAgentToWorld.cc
)

add_class(BOLDHUMANOID
  ./StateObject/AgentFrameState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/AmbulatorState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/BodyControlState/BodyControlState.cc
  ./StateObject/BodyControlState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/BodyState/BodyState.cc
  ./StateObject/BodyState/initBody.cc
  ./StateObject/BodyState/updatePosture.cc
  ./StateObject/BodyState/writeJson.cc
  ./StateObject/BodyState/zero.cc
)

add_class(BOLDHUMANOID
  ./StateObject/CameraFrameState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/DebugState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/GameState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/HardwareState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/LabelCountState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/MotionTaskState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/OptionTreeState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/ParticleState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/StaticHardwareState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObject/WorldFrameState/writeJson.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/FallDetector/falldetector.cc
)

add_class(BOLDHUMANOID
  ./StateObserver/GyroCalibrator/gyrocalibrator.cc
)

add_class(BOLDHUMANOID
  ./TeamCommunicator/teamcommunicator.cc
)

add_class(BOLDHUMANOID
  ./ThreadId/statics.cc
)

add_class(BOLDHUMANOID
  ./UDPSocket/udpsocket.cc
)

add_class(BOLDHUMANOID
  ./VisualCortex/VisualCortex.cc
  ./VisualCortex/integrateImage.cc
  ./VisualCortex/shouldMergeBallBlobs.cc
  ./VisualCortex/saveImage.cc
  ./VisualCortex/streamDebugImage.cc
)

add_class(BOLDHUMANOID
  ./Voice/voice.cc
)

add_class(BOLDHUMANOID
  ./util/ccolor.cc
  ./util/log.cc
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
set(BOLDHUMANOID_LINK_LIBRARIES ${OpenCV_LIBS} ${SigC++_LIBRARIES} websockets pthread rt espeak)
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
add_dependencies(boldhumanoid libwebsockets_lib)


set(BOLDHUMANOID_DEPLOY_FILES
  $<TARGET_FILE:boldhumanoid>
  ${CMAKE_SOURCE_DIR}/*.ini
  ${CMAKE_SOURCE_DIR}/www
  ${CMAKE_SOURCE_DIR}/motion_4096.bin
)

#
## Upload target
#
if(UPLOAD_SCP_PATH)
  add_custom_command(OUTPUT upload.stamp
    COMMAND rsync -rzP ${BOLDHUMANOID_DEPLOY_FILES} ${UPLOAD_SCP_PATH}
    COMMAND touch upload.stamp
    DEPENDS boldhumanoid)

  add_custom_target(upload DEPENDS upload.stamp)
else(UPLOAD_SCP_PATH)
  message(WARNING "You have not set a path for uploading the binary; 'upload' target not generated")
endif(UPLOAD_SCP_PATH)

#
## Bundle target
#
if(BUNDLES_PATH)
  add_custom_command(OUTPUT bundle.stamp
    COMMAND if [ \"\${TAG}\" != \"\" ]\;
          then
            git tag \${TAG}\;
            mkdir -p ${BUNDLES_PATH}/\${TAG}\;
            cp -r ${BOLDHUMANOID_DEPLOY_FILES} ${BUNDLES_PATH}/\${TAG}\;
          else
            echo \"No tag given. Usage: make bundle TAG=<tag>\"\;
          fi
    COMMAND touch bundle.stamp
    DEPENDS boldhumanoid

    )

  add_custom_target(bundle DEPENDS bundle.stamp)
else(BUNDLES_PATH)
  message(WARNING "You have not set a path for storing bundles; 'bundle' target not generated")
endif(BUNDLES_PATH)


#
## CTAGS target (for emacs users)
#
set(_command find ${CMAKE_SOURCE_DIR} -regextype posix-egrep -regex ".*((hh)|(cc))$" -print | etags -o ${CMAKE_SOURCE_DIR}/TAGS -)
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/TAGS
  COMMAND ${_command}
  IMPLICIT_DEPENDS ${BOLDHUMANOID_SORCES}
  VERBATIM
  )
add_custom_target(tags DEPENDS ${CMAKE_SOURCE_DIR}/TAGS)

#
## SWIG/Python
#
add_subdirectory(swig)

#
## Build and run test
#
if (BUILDRUN_UNIT_TEST)
  add_subdirectory(test)
endif (BUILDRUN_UNIT_TEST)

#
## Build tools
#
if (BUILD_TOOLS)
  add_subdirectory(tools)
endif (BUILD_TOOLS)

